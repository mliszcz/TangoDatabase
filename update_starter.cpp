//=============================================================================
//
// file :        update_starter.cpp
//
// description : thread to inform Strater to update from database
//                  in case of controlled servers list modification.
//
// project :	Starter for Tango Administration
//
// $Author$
//
// Copyright (C) :      2004,2005,2006,2007,2008,2009,2010,2011,2012,2013
//						European Synchrotron Radiation Facility
//                      BP 220, Grenoble 38043
//                      FRANCE
//
// This file is part of Tango.
//
// Tango is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// Tango is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with Tango.  If not, see <http://www.gnu.org/licenses/>.
//
// $Revision$
// $Date$
//
// $HeadURL:$
//
//=============================================================================


#include <DataBase.h>

namespace DataBase_ns
{
//=============================================================================
//=============================================================================
UpdStarterData::UpdStarterData(string domain)
{
    starter_header = domain;
	starter_header += STARTER_DEVNAME_FAMILY;
}
//=============================================================================
//=============================================================================
vector<string> UpdStarterData::get_starter_devname()
{
	omni_mutex_lock sync(*this);
	return starter_devnames;
}
//=============================================================================
//=============================================================================
string UpdStarterData::get_starter_header()
{
	omni_mutex_lock sync(*this);
	return starter_header;
}
//=============================================================================
//=============================================================================
void UpdStarterData::send_starter_cmd(vector<string> hostnames)
{
	omni_mutex_lock sync(*this);

	//	Build starter devices to update
	starter_devnames.clear();
	for (unsigned int i=0 ; i<hostnames.size() ; i++)
	{
		string	devname(starter_header);
		devname += hostnames[i];
		starter_devnames.push_back(devname);
	}

	//	Awake thread
	signal();
}


//=============================================================================
//=============================================================================
UpdateStarter::UpdateStarter(UpdStarterData *sd)
{
	shared = sd;
}
//=============================================================================
//=============================================================================
void *UpdateStarter::run_undetached(TANGO_UNUSED(void *ptr))
{
	while(true)
	{
		//	Get the starter device name
		vector<string>	devnames = shared->get_starter_devname();
        string starter_header = shared->get_starter_header();
		for (unsigned int i=0 ; i<devnames.size() ; i++)
		{
			//	Verify if devname has been set
			if (devnames[i].find(starter_header)==0)
			{
				//	Remove the Fully Qualify Domain Name of host for device name
				string::size_type	pos = devnames[i].find('.');
				if (pos != string::npos)
					 devnames[i] = devnames[i].substr(0, pos);
				//cout << "Fire info to " << devnames[i] << endl;

				Tango::DeviceProxy	*dev = NULL;
				try
				{
					//	Build connection and send command
					dev = new Tango::DeviceProxy(devnames[i]);
					cout << devnames[i] << " imported" << endl;
					dev->command_inout("UpdateServersInfo");
					cout << "dev->command_inout(UpdateServersInfo) sent to " << devnames[i] << endl;
				}
				catch(Tango::DevFailed &e)
				{
					//Tango::Except::print_exception(e);
					cout << e.errors[0].desc << endl;
				}
				delete dev;
			}
		}
		//	Wait until next command.
		{
			omni_mutex_lock sync(*shared);
			shared->wait();
		}
		cout2 << "Thread update_starter awaken !" << endl;
		
	}
	return NULL;
}
//=============================================================================
//=============================================================================

}	//	namespace
