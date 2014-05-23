#ifndef _LIVEMEDIA_HH
#include <liveMedia/liveMedia.hh>
#endif

#include <string>

#ifndef _HANDLERS_HH
#include "../src/network/Handlers.hh"
#endif

#ifndef _SOURCE_MANAGER_HH
#include "../src/network/SourceManager.hh"
#endif

#include "../src/AudioFrame.hh"
#include "../src/modules/audioDecoder/AudioDecoderLibav.hh"
#include "../src/modules/audioEncoder/AudioEncoderLibav.hh"
#include "../src/modules/audioMixer/AudioMixer.hh"
#include "../src/AudioCircularBuffer.hh"

#include <iostream>
#include <csignal>


#define PROTOCOL "RTP"
#define PAYLOAD 97
#define BANDWITH 5000

#define A_CODEC "opus"
#define A_CLIENT_PORT1 6006
#define A_CLIENT_PORT2 6008
#define A_MEDIUM "audio"
#define A_TIME_STMP_FREQ 48000
#define A_CHANNELS 2

#define CHANNEL_MAX_SAMPLES 3000
#define OUT_CHANNELS 2
#define OUT_SAMPLE_RATE 48000

bool should_stop = false;

struct buffer {
    unsigned char* data;
    int data_len;
};

void signalHandler( int signum )
{
    std::cout << "Interrupt signal (" << signum << ") received.\n";
    
    SourceManager *mngr = SourceManager::getInstance();
    mngr->closeManager();
    
    std::cout << "Manager closed\n";
}

void fillBuffer(struct buffer *b, Frame *pFrame) {
    memcpy(b->data + b->data_len, pFrame->getDataBuf(), pFrame->getLength());
    b->data_len += pFrame->getLength(); 
}

void saveBuffer(struct buffer *b) 
{
    FILE *audioChannel = NULL;
    char filename[32];

    sprintf(filename, "coded.opus");

    audioChannel = fopen(filename, "wb");

    if (b->data != NULL) {
        fwrite(b->data, b->data_len, 1, audioChannel);
    }

    fclose(audioChannel);
}

void readingRoutine(struct buffer* b, AudioCircularBuffer* cb1,  AudioCircularBuffer* cb2, AudioEncoderLibav* enc)
{
    std::map<int,Frame*> mapFrame;
    
    AudioMixer *mixer = new AudioMixer(4);

    InterleavedAudioFrame* codedFrame = InterleavedAudioFrame::createNew (
                                                    OUT_CHANNELS, 
                                                    OUT_SAMPLE_RATE, 
                                                    CHANNEL_MAX_SAMPLES, 
                                                    enc->getCodec(), 
                                                    S16
                                                  );

    PlanarAudioFrame* mixedFrame = PlanarAudioFrame::createNew (
                                                    OUT_CHANNELS, 
                                                    OUT_SAMPLE_RATE, 
                                                    CHANNEL_MAX_SAMPLES, 
                                                    PCM, 
                                                    S16P
                                                  );

    cb1->setOutputFrameSamples(DEFAULT_FRAME_SAMPLES);
    cb2->setOutputFrameSamples(DEFAULT_FRAME_SAMPLES);

    while(!should_stop) {
        mapFrame[0] = cb1->getFront();
        mapFrame[1] = cb2->getFront();

        if(!mapFrame[0] || !mapFrame[1]) {
            usleep(500);
            continue;
        }

        //mixer->mix(mapFrame[1], mapFrame[2], mixedFrame);
        mixer->doProcessFrame(mapFrame, mixedFrame);

        cb1->removeFrame();
        cb2->removeFrame();

        if(!enc->doProcessFrame(mixedFrame, codedFrame)) {
           std::cerr << "Error encoding frame" << std::endl;
        }

        fillBuffer(b, codedFrame);
        printf("Filled buffer! Frame size: %d\n", codedFrame->getLength());
    }

    saveBuffer(b);
    printf("Buffer saved\n");
}

int main(int argc, char** argv) 
{   
    std::string sessionId;
    std::string sdp;
    Session* session;
    SourceManager *mngr = SourceManager::getInstance();
    AudioDecoderLibav* audioDecoder1;
    AudioDecoderLibav* audioDecoder2;
    AudioEncoderLibav* audioEncoder;
    std::map<unsigned short, FrameQueue*> inputs;
    FrameQueue* q1;
    FrameQueue* q2;
    AudioCircularBuffer* audioCirBuffer1;
    AudioCircularBuffer* audioCirBuffer2;
    Frame* aFrame;
    PlanarAudioFrame* destinationPlanarFrame;
    Frame *codedFrame;
    struct buffer *buffers;

    ACodecType inCType = OPUS;
    ACodecType outCType = PCM;
    SampleFmt outSFmt = S16P;
    unsigned int bytesPerSample = 2;

    
    signal(SIGINT, signalHandler); 
    
    for (int i = 1; i <= argc-1; ++i) {
        sessionId = handlers::randomIdGenerator(ID_LENGTH);
        session = Session::createNewByURL(*(mngr->envir()), argv[0], argv[i]);
        mngr->addSession(sessionId, session);
    }
    
    sessionId = handlers::randomIdGenerator(ID_LENGTH);
    
    sdp = handlers::makeSessionSDP("testSession", "this is a test");
    
    sdp += handlers::makeSubsessionSDP(A_MEDIUM, PROTOCOL, PAYLOAD, A_CODEC, BANDWITH, 
                                        A_TIME_STMP_FREQ, A_CLIENT_PORT1, A_CHANNELS);
    sdp += handlers::makeSubsessionSDP(A_MEDIUM, PROTOCOL, PAYLOAD, A_CODEC, BANDWITH, 
                                        A_TIME_STMP_FREQ, A_CLIENT_PORT2, A_CHANNELS);
    
    session = Session::createNew(*(mngr->envir()), sdp);
    
    mngr->addSession(sessionId, session);
       
    mngr->runManager();
       
    mngr->initiateAll();

    audioDecoder1 = new AudioDecoderLibav();
    audioDecoder2 = new AudioDecoderLibav();

    audioEncoder = new AudioEncoderLibav();
    audioEncoder->configure(PCMU);

    audioCirBuffer1 = AudioCircularBuffer::createNew(OUT_CHANNELS, OUT_SAMPLE_RATE, CHANNEL_MAX_SAMPLES, outSFmt);
    audioCirBuffer2 = AudioCircularBuffer::createNew(OUT_CHANNELS, OUT_SAMPLE_RATE, CHANNEL_MAX_SAMPLES, outSFmt);

    inputs = mngr->getInputs();
    q1 = inputs[A_CLIENT_PORT1];
    q2 = inputs[A_CLIENT_PORT2];

    buffers = new struct buffer;
    buffers->data = new unsigned char[CHANNEL_MAX_SAMPLES * bytesPerSample * OUT_SAMPLE_RATE * 360]();
    buffers->data_len = 0;
    
    std::thread readingThread(readingRoutine, buffers, audioCirBuffer1, audioCirBuffer2, audioEncoder);

    while(mngr->isRunning()) {
        if ((codedFrame = q1->getFront()) != NULL) {

            aFrame = audioCirBuffer1->getRear();

            if(!audioDecoder1->doProcessFrame(codedFrame, aFrame)) {
                std::cout << "Error decoding frames\n";
            }

            q1->removeFrame();
            audioCirBuffer1->addFrame();
        }

        if ((codedFrame = q2->getFront()) != NULL) {

            aFrame = audioCirBuffer2->getRear();

            if(!audioDecoder2->doProcessFrame(codedFrame, aFrame)) {
                std::cout << "Error decoding frames\n";
            }

            q2->removeFrame();
            audioCirBuffer2->addFrame();
        }

        if (codedFrame == NULL) {
            usleep(500);
        }
    }

    should_stop = true;
    readingThread.join();
    
    return 0;
}