/* +---------------------------------------------------------------------------+
   |                 The Mobile Robot Programming Toolkit (MRPT)               |
   |                                                                           |
   |                          http://www.mrpt.org/                             |
   |                                                                           |
   | Copyright (c) 2005-2013, Individual contributors, see AUTHORS file        |
   | Copyright (c) 2005-2013, MAPIR group, University of Malaga                |
   | Copyright (c) 2012-2013, University of Almeria                            |
   | All rights reserved.                                                      |
   |                                                                           |
   | Redistribution and use in source and binary forms, with or without        |
   | modification, are permitted provided that the following conditions are    |
   | met:                                                                      |
   |    * Redistributions of source code must retain the above copyright       |
   |      notice, this list of conditions and the following disclaimer.        |
   |    * Redistributions in binary form must reproduce the above copyright    |
   |      notice, this list of conditions and the following disclaimer in the  |
   |      documentation and/or other materials provided with the distribution. |
   |    * Neither the name of the copyright holders nor the                    |
   |      names of its contributors may be used to endorse or promote products |
   |      derived from this software without specific prior written permission.|
   |                                                                           |
   | THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS       |
   | 'AS IS' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED |
   | TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR|
   | PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDERS BE LIABLE |
   | FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL|
   | DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR|
   |  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)       |
   | HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,       |
   | STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN  |
   | ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           |
   | POSSIBILITY OF SUCH DAMAGE.                                               |
   +---------------------------------------------------------------------------+ */



#include <mrpt/hwdrivers.h>  // Precompiled headers

#include <mrpt/hwdrivers/CImpinjRFID.h>

#include <string.h>

#include <stdlib.h>


using namespace mrpt::hwdrivers;

IMPLEMENTS_GENERIC_SENSOR(CImpinjRFID,mrpt::hwdrivers)

CImpinjRFID::CImpinjRFID()
{
	m_sensorLabel = "RFID";
	connected = false;
}


CImpinjRFID::~CImpinjRFID()
{
	closeReader();
}


void CImpinjRFID::initialize()
{


	// start the driver
	//use a separate thread so the connection can be established while the program starts. This is essential because the module will stall on the accept() call until the driver executable requests a connection, and, on the other hand, if the module is not listening, the driver will fail
	mrpt::system::createThread(dummy_startDriver,this);
//	system::createThreadFromObjectMethod<CImpinjRFID>(this,startDriver);

	// start connection

	connect();

}



void CImpinjRFID::dummy_startDriver(CImpinjRFID *o)
{
	o->startDriver();
}

void CImpinjRFID::startDriver()
{
	// start the driver
	std::stringstream cmdline;
	std::cout << "Waiting for the driver to start ... ";

	// create the command line (executable path + parameters)
	cmdline << driver_path << " " << reader_name.c_str() << " " << IPm.c_str() << " " << port;

	// wait until the current module starts the sockets and listens to it
	system::sleep(2000);

	const int ret = ::system(cmdline.str().c_str());
	if (0!=ret)
		std::cerr << "[CImpinjRFID::startDriver] Error ("<< ret << ") invoking command:\n" << cmdline << std::endl;

	system::exitThread();  // JL->Emil: Really needed? If not, just remove...
}

void  CImpinjRFID::loadConfig_sensorSpecific(
	const mrpt::utils::CConfigFileBase &configSource,
	const std::string			&iniSection )
{

	MRPT_START
		// TEMPORARILY DISABLED
/*		pose_x_1 = configSource.read_float(iniSection,"pose_x_1",0,true);
		pose_y_1 = configSource.read_float(iniSection,"pose_y_1",0,true);
		pose_z_1 = configSource.read_float(iniSection,"pose_z_1",0,true);
		pose_roll_1 = configSource.read_float(iniSection,"pose_roll_1",0,true);
		pose_pitch_1 = configSource.read_float(iniSection,"pose_pitch_1",0,true);
		pose_yaw_1 = configSource.read_float(iniSection,"pose_yaw_1",0,true);

		pose_x_2 = configSource.read_float(iniSection,"pose_x_2",0,true);
		pose_y_2 = configSource.read_float(iniSection,"pose_y_2",0,true);
		pose_z_2 = configSource.read_float(iniSection,"pose_z_2",0,true);
		pose_roll_2 = configSource.read_float(iniSection,"pose_roll_2",0,true);
		pose_pitch_2 = configSource.read_float(iniSection,"pose_pitch_2",0,true);
		pose_yaw_2 = configSource.read_float(iniSection,"pose_yaw_2",0,true);
*/
		IPm = configSource.read_string(iniSection,"local_IP","127.0.0.1",false);
		reader_name = configSource.read_string(iniSection,"reader_name","", true);
		port = configSource.read_int(iniSection,"listen_port",0,true);
		driver_path = configSource.read_string(iniSection,"driver_path","",true);

	MRPT_END
}

/*---------------------------------------------------------------
					getObservation
    Get the power of a given network as an observation
 ---------------------------------------------------------------*/
void CImpinjRFID::connect()
{
	if (!connected)

		// Start the server
		server = new mrpt::utils::CServerTCPSocket(port);

	client = server->accept();

	system::sleep(1000);
	connected = true;

}

/*---------------------------------------------------------------
					getObservation
    Get the power of a given network as an observation
 ---------------------------------------------------------------*/
bool CImpinjRFID::getObservation( mrpt::slam::CObservationRFID &obs)
{
	try{
		bool receivedSomething = false;
		char msg[34];
		char cmd[20];
		char epc[24];
		char rx_pwr[5];
		char *tmp;
		obs.tag_readings.clear();
		// send an observation command to the device interface program
		strcpy(cmd, "OBS\0");
		client->writeAsync(cmd,10);

		// receive a reading from the sensor through the socket
		while (client->readAsync(msg,34,100) > 0)
		{
			receivedSomething = true;
			// the received string is in the format: ANT_PORT EPC RX_PWR ZERO_FILL
			const char *ant_port_ptr = strtok(msg," ",&tmp);
			if (!ant_port_ptr)
			{
			    std::cerr << "[CImpinjRFID::getObservation] Unexpected format in sensor data! (skipping).\n";
			    continue;
			}
			const char ant_port =  *ant_port_ptr;
			strcpy(epc,strtok(NULL," ",&tmp));
			strcpy(rx_pwr,strtok(NULL, " ",&tmp));

			// Fill the observation
			obs.tag_readings.resize(obs.tag_readings.size()+1);  // Alloc space for one more tag obs
			mrpt::slam::CObservationRFID::TTagReading & new_tag = *obs.tag_readings.rbegin();  // Get a reference to the latest new tag structure

			// Fill in fields in "new_tag":
			new_tag.antennaPort = mrpt::format("%c",ant_port);
			new_tag.epc = std::string(epc);
			new_tag.power = atof(rx_pwr);
			obs.sensorLabel = m_sensorLabel;

			//std::cout << "mrpt::hwdrivers::CImpinjRFID::getObservation() " << "\n\tRXPWR: " << atof(rx_pwr) << " PWR READ: " << rx_pwr << std::endl;
		}
		if (receivedSomething)
			return true;
		else
			return false;

	}
	catch (std::exception &e)
	{
		cerr << e.what() << endl;
		return false;
	}
}

/*---------------------------------------------------------------
					getObservation
    Get the power of a given network as an observation
 ---------------------------------------------------------------*/
void CImpinjRFID::closeReader()
{
	char cmd[20];
	// send a kill command to the device interface program
	strcpy(cmd, "END\0");
	client->writeAsync(cmd,10);  // JL->Emil: Why 10 not strlen(cmd)? Is the server expecting a fixed length data block?
	client->close();
}


void CImpinjRFID::doProcess(){

	mrpt::slam::CObservationRFIDPtr obs = mrpt::slam::CObservationRFID::Create();
	if(getObservation(*obs))
		appendObservation(obs);
}
