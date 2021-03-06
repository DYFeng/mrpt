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

#include <mrpt/base.h>  // Precompiled headers


#include <mrpt/utils/CSerializable.h>
#include <mrpt/utils/CMemoryStream.h>
#include <mrpt/system/os.h>

using namespace mrpt;
using namespace mrpt::utils;
using namespace mrpt::system;

#include <iostream>


IMPLEMENTS_VIRTUAL_MRPT_OBJECT(CSerializable, CObject, mrpt::utils)


/* -----------------------------------------------------------------------
	Used to pass MRPT objects into a CORBA-like object,
		a string. See doc about "Integration with BABEL".
   ----------------------------------------------------------------------- */
std::string utils::ObjectToString(const CSerializable *o)
{
	CMemoryStream				tmp,tmpCoded;
	size_t						n;
	std::string					str;

	try
	{
		tmp.WriteObject(o);
		n = tmp.getTotalBytesCount();

		// Scan the string to code it:
		// ----------------------------------
		int				lastIdx = 0;
		unsigned char	*data = (unsigned char*)tmp.getRawBufferData();
		unsigned char	c;
		for (size_t i=0;i<n;i++)
		{
			c = data[i];
			// Search for first "0x00" byte:
			if ( c == 0x01 || !c )
			{
				// Copy all till now:
				tmpCoded.WriteBuffer( &data[lastIdx], i - lastIdx);
				lastIdx = i+1;

				// And code:
				//   0x01 --> 0x01 0x01
				//   0x00 --> 0x01 0x02
				unsigned char dumm[2];
				dumm[0] = 0x01;
				if (c)
						dumm[1] = 0x01;
				else	dumm[1] = 0x02;

				// Append to coded stream:
				tmpCoded.WriteBuffer( dumm, 2);
			}
		} // end for i

		// Copy the rest:
		if ( (n-lastIdx) > 0)
			tmpCoded.WriteBuffer( &data[lastIdx], n - lastIdx );

		// Copy to string object:
		n = tmpCoded.getTotalBytesCount();
		str.resize(n);
		memcpy(&str[0],tmpCoded.getRawBufferData(),n);
		return str;
	}
	catch (std::bad_alloc &e)
	{
		throw e;
	}
	catch(std::exception &e)
	{
		std::cerr << "[ObjectToString] Exception: " << e.what() << std::endl;
		return "";
	}
	catch(...)
	{
		std::cerr << "[ObjectToString] Unknown exception" << std::endl;
		return "";
	}
}


/* -----------------------------------------------------------------------
	Used to pass CORBA-like object into a MRPT object.
		See doc about "Integration with BABEL".
   ----------------------------------------------------------------------- */
void utils::StringToObject(const std::string &str, CSerializablePtr &obj)
{
	MRPT_START

	obj.clear_unique();
	if (str.empty()) return;

	CMemoryStream	tmp;
	size_t			n;
	size_t			i,lastIdx;

	obj.clear_unique();

	n = str.size();

	// Scan the string to decode it:
	// ----------------------------------
	lastIdx = 0;
	const char *data = str.c_str();
	unsigned char c;
	for (i=0;i<n && (c=data[i])!=0;i++)
	{
		// Search for first "0x01" byte:
		if ( c == 0x01 )
		{
			// Copy all till now:
			tmp.WriteBuffer( &data[lastIdx], i - lastIdx + 1 );
			i+=1; // +1 from "for" loop
			lastIdx = i+1;

			// And decode:
			//   0x01 0x01 --> 0x01
			//   0x01 0x02 --> 0x00
			if (data[i]==0x01)
					((unsigned char*)tmp.getRawBufferData())[tmp.getTotalBytesCount()-1] = (unsigned char)0x01;
			else 	((unsigned char*)tmp.getRawBufferData())[tmp.getTotalBytesCount()-1] = (unsigned char)0x00;
		}
	} // end for i

	// Copy the rest:
	if ( (n-lastIdx) > 0)
		tmp.WriteBuffer( &data[lastIdx], n - lastIdx );

	// And the '\0' char:
	char dummy = '\0';
	tmp.WriteBuffer( &dummy, sizeof(char) );

	tmp.Seek(0,CStream::sFromBeginning);
	obj = tmp.ReadObject();

	MRPT_END

}


/* -----------------------------------------------------------------------
				ObjectToOctetVector
   ----------------------------------------------------------------------- */
void utils::ObjectToOctetVector(const CSerializable *o, vector_byte & out_vector)
{
	try
	{
		CMemoryStream	tmp;
		tmp.WriteObject(o);

		size_t N = tmp.getTotalBytesCount();
		out_vector.resize(N);
		if (N)
		{
			os::memcpy( &out_vector[0],N,tmp.getRawBufferData(), N );
		}
	}
	catch (std::bad_alloc &e)
	{
		throw e;
	}
	catch(std::exception &e)
	{
		std::cerr << "[ObjectToOctetVector] Exception: " << e.what() << std::endl;
	}
	catch(...)
	{
		std::cerr << "[ObjectToOctetVector] Unknown exception" << std::endl;
	}
}

/* -----------------------------------------------------------------------
				OctetVectorToObject
   ----------------------------------------------------------------------- */
void utils::OctetVectorToObject(const vector_byte & in_data, CSerializablePtr &obj)
{
	try
	{
		obj.clear_unique();

		if (in_data.empty()) return;

		CMemoryStream	tmp( &in_data[0], in_data.size());
		obj = tmp.ReadObject();
	}
	catch (std::bad_alloc &e)
	{
		throw e;
	}
	catch(std::exception &e)
	{
		std::cerr << "[OctetVectorToObject] Exception: " << e.what() << std::endl;
	}
	catch(...)
	{
		std::cerr << "[OctetVectorToObject] Unknown exception" << std::endl;
	}
}

/* -----------------------------------------------------------------------
				ObjectToRawString
   ----------------------------------------------------------------------- */
void utils::ObjectToRawString(const CSerializable *o, std::string & out_vector)
{
	try
	{
		CMemoryStream	tmp;
		tmp.WriteObject(o);

		size_t N = tmp.getTotalBytesCount();
		out_vector.resize(N);
		if (N)
		{
			os::memcpy( &out_vector[0],N,tmp.getRawBufferData(), N );
		}
	}
	catch (std::bad_alloc &e)
	{
		throw e;
	}
	catch(std::exception &e)
	{
		std::cerr << "[ObjectToOctetVector] Exception: " << e.what() << std::endl;
	}
	catch(...)
	{
		std::cerr << "[ObjectToOctetVector] Unknown exception" << std::endl;
	}
}

/* -----------------------------------------------------------------------
				RawStringToObject
   ----------------------------------------------------------------------- */
void utils::RawStringToObject(const std::string & in_data, CSerializablePtr &obj)
{
	try
	{
		obj.clear_unique();

		if (in_data.empty()) return;

		CMemoryStream	tmp( &in_data[0], in_data.size());
		obj = tmp.ReadObject();
	}
	catch (std::bad_alloc &e)
	{
		throw e;
	}
	catch(std::exception &e)
	{
		std::cerr << "[OctetVectorToObject] Exception: " << e.what() << std::endl;
	}
	catch(...)
	{
		std::cerr << "[OctetVectorToObject] Unknown exception" << std::endl;
	}
}


