/*
 *  Utils.hh - Different utils
 *  Copyright (C) 2014  Fundació i2CAT, Internet i Innovació digital a Catalunya
 *
 *  This file is part of liveMediaStreamer.
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 *  Authors:  Marc Palau <marc.palau@i2cat.net>
 *            David Cassany <david.cassany@i2cat.net>
 */

#include "Utils.hh"

namespace utils 
{
    SampleFmt getSampleFormatFromString(std::string stringSampleFmt)
    {
        SampleFmt sampleFormat;
        
        if (stringSampleFmt.compare("u8") == 0) {
            sampleFormat = U8;
        } else if (stringSampleFmt.compare("u8p") == 0) {
            sampleFormat = U8P;
        }  else if (stringSampleFmt.compare("s16") == 0) {
            sampleFormat = S16;
        }  else if (stringSampleFmt.compare("s16p") == 0) {
            sampleFormat = S16P;
        }  else if (stringSampleFmt.compare("flt") == 0) {
            sampleFormat = FLT;
        }  else if (stringSampleFmt.compare("fltp") == 0) {
            sampleFormat = FLTP;
        }  else {
            sampleFormat = S_NONE;
        }

        return sampleFormat;
    }

    ACodecType getCodecFromString(std::string stringCodec)
    {
        ACodecType codec;
        
        if (stringCodec.compare("g711") == 0) {
            codec = G711;
        } else if (stringCodec.compare("pcmu") == 0) {
            codec = PCMU;
        }  else if (stringCodec.compare("opus") == 0) {
            codec = OPUS;
        }  else if (stringCodec.compare("pcm") == 0) {
            codec = PCM;
        }  else if (stringCodec.compare("aac") == 0) {
            codec = AAC;
        }  else if (stringCodec.compare("mp3") == 0) {
            codec = MP3;
        }  else {
            codec = AC_NONE;
        }

        return codec;
    }

    std::string getAudioCodecAsString(ACodecType codec)
    {
       std::string stringCodec;

        switch(codec) {
            case G711:
                stringCodec = "g711";
                break;
            case PCMU:
                stringCodec = "pcmu";
                break;
            case OPUS:
                stringCodec = "opus";
                break;
            case PCM:
                stringCodec = "pcm";
                break;
            case AAC:
                stringCodec = "aac";
                break;
            case MP3:
                stringCodec = "mp3";
                break;
        }

        return stringCodec;
    }

    std::string getFilterTypeAsString(FilterType type) 
    {
        std::string stringType;

        switch(type) {
            case VIDEO_DECODER:
                stringType = "videoDecoder";
                break;
            case VIDEO_ENCODER:
                stringType = "videoEncoder";
                break;
            case VIDEO_MIXER:
                stringType = "videoMixer";
                break;
            case AUDIO_DECODER:
                stringType = "audioDecoder";
                break;
            case AUDIO_ENCODER:
                stringType = "audioEncoder";
                break;
            case AUDIO_MIXER:
                stringType = "audioMixer";
                break;
        }

        return stringType;
    }

    std::string getSampleFormatAsString(SampleFmt sFormat) 
    {
        std::string stringFormat;

        switch(sFormat) {
            case U8:
                stringFormat = "u8";
                break;
            case S16:
                stringFormat = "s16";
                break;
            case FLT:
                stringFormat = "flt";
                break;
            case U8P:
                stringFormat = "u8p";
                break;
            case S16P:
                stringFormat = "16p";
                break;
            case FLTP:
                stringFormat = "fltp";
                break;
        }

        return stringFormat;

    }
}