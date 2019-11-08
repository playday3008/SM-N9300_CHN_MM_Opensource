/*
Copyright (C) 2004-2008 Grame

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation; either version 2.1 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.

*/

#include "JackControlLibClient.h"

namespace Jack
{

JackControlLibClient::JackControlLibClient(JackSynchro* table): JackLibClient(table)
{
}

JackControlLibClient::~JackControlLibClient()
{
}

#if defined (__ANDROID__)
int JackControlLibClient::StopJackd(int param){
	jack_log("JackClientLibClient::StopJackd");

    int result = -1;
    fChannel->ClientStopJackd(GetClientControl()->fRefNum, param, &result);
    return result;
}

int JackControlLibClient::SwitchMaster(int type, int param){
	jack_log("JackClientLibClient::SwitchMaster");

    int result = -1;
    fChannel->ClientSwitchMaster(GetClientControl()->fRefNum, type, param, &result);
    return result;
}
#endif

#if defined (__ANDROID__) && defined (ENABLE_JACK_LOGGER)
int JackControlLibClient::StartDump(int type, int param){
	jack_log("JackClientLibClient::StartDump");

    int result = -1;
    fChannel->ClientStartDump(GetClientControl()->fRefNum, type, param, &result);
    return result;
}

int JackControlLibClient::StopDump(int param){
	jack_log("JackClientLibClient::StopDump");

    int result = -1;
    fChannel->ClientStopDump(GetClientControl()->fRefNum, param, &result);
    return result;
}

int JackControlLibClient::Dump(int param){
	jack_log("JackClientLibClient::Dump");

    int result = -1;
    fChannel->ClientDump(GetClientControl()->fRefNum, param, &result);
    return result;
}
#endif


} // end of namespace



