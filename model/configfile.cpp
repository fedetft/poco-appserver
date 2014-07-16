/***************************************************************************
 *   Copyright (C) 2010 by Terraneo Federico                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   As a special exception, if other files instantiate templates or use   *
 *   macros or inline functions from this file, or you compile this file   *
 *   and link it with other works to produce a work based on this file,    *
 *   this file does not by itself cause the resulting work to be covered   *
 *   by the GNU General Public License. However the source code for this   *
 *   file must still be made available in accordance with the GNU General  *
 *   Public License. This exception does not invalidate any other reasons  *
 *   why a work based on this file might be covered by the GNU General     *
 *   Public License.                                                       *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, see <http://www.gnu.org/licenses/>   *
 ***************************************************************************/

#include "configfile.h"
#include <iostream>
#include <fstream>
#include <stdexcept>

using namespace std;

ConfigFile::ConfigFile(string configfile)
{
	ifstream conf(configfile.c_str());
	if(!conf.good())
	{
		throw(runtime_error(string("Can't find config file ")+configfile));
	}
	string line;
	while(getline(conf,line))
	{
		if(line.length()==0) continue;
		if(line.at(0)=='#') continue;
		size_t equal=line.find('=');
		if(equal==string::npos || equal==line.length()-1)
		{
			cerr<<"bogus line in \""<<configfile<<"\" : \""<<line<<"\""<<endl;
			continue;
		}
		string option=line.substr(0,equal);
		string value=line.substr(equal+1);
		//cout<<"adding tuple "<<option<<":"<<value<<endl;
		map<string,string>::const_iterator it=options.find(option);
		if(it!=options.end())
		{
			cerr<<"line \""<<line<<"\" in config file \""<<configfile
				<<"\" redeclares a previous option, ignoring"<<endl;
			continue;
		}
		options[option]=value;
	}
}

bool ConfigFile::hasOption(string option)
{
	map<string,string>::const_iterator it=options.find(option);
	return it!=options.end();
}

string ConfigFile::getString(string option)
{
	map<string,string>::const_iterator it=options.find(option);
	if(it==options.end())
	{
		throw(runtime_error(string("Option not found: ")+option));
	}
	return it->second;
}
