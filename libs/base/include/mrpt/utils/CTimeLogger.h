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
#ifndef  CTimeLogger_H
#define  CTimeLogger_H

#include <mrpt/utils/CTicTac.h>
#include <mrpt/utils/CDebugOutputCapable.h>

#include <stack>

namespace mrpt
{
	namespace utils
	{
		using namespace std;

		/** A versatile "profiler" that logs the time spent within each pair of calls to enter(X)-leave(X), among other stats.
		 *  The results can be dumped to cout or to Visual Studio's output panel.
		 *  Recursive methods are supported with no problems, that is, calling "enter(X) enter(X) ... leave(X) leave(X)".
		 *
		 *  This class can be also used to monitorize min/mean/max/total stats of any user-provided parameters via the method CTimeLogger::registerUserMeasure()
		 *
		 * \sa CTimeLoggerEntry
		 * 
		 * \note The default behavior is dumping all the information at destruction.
		 * \ingroup mrpt_base_grp
		 */
		class BASE_IMPEXP CTimeLogger : public mrpt::utils::CDebugOutputCapable
		{
		private:
			CTicTac		m_tictac;
			bool		m_enabled;

			//! Data of all the calls:
			struct TCallData
			{
				TCallData();

				size_t n_calls;
				double min_t,max_t,mean_t;
				stack<double,vector<double> >   open_calls;
				bool has_time_units;
			};

			map<string,TCallData>  m_data;

			void do_enter( const char *func_name );
			double do_leave( const char *func_name );

		public:
			/** Data of each call section: # of calls, minimum, maximum, average and overall execution time (in seconds) \sa getStats */
			struct TCallStats
			{
				size_t n_calls;
				double min_t,max_t,mean_t,total_t;
			};

			CTimeLogger(bool enabled = true); //! Default constructor
			virtual ~CTimeLogger(); //!< Destructor
			std::string getStatsAsText(const size_t column_width=80) const; //!< Dump all stats to a multi-line text string. \sa dumpAllStats, saveToCVSFile
			void getStats(std::map<std::string,TCallStats> &out_stats) const; //!< Returns all the current stats as a map: section_name => stats. \sa getStatsAsText, dumpAllStats, saveToCVSFile
			void dumpAllStats(const size_t column_width=80) const; //!< Dump all stats through the CDebugOutputCapable interface. \sa getStatsAsText, saveToCVSFile
			void clear(bool deep_clear=false); //!< Resets all stats. By default (deep_clear=false), all section names are remembered (not freed) so the cost of creating upon the first next call is avoided.
			void enable(bool enabled = true) { m_enabled = enabled; }
			void disable() { m_enabled = false; }
			void saveToCSVFile(const std::string &csv_file)  const; 	//!< Dump all stats to a Comma Separated Values (CSV) file. \sa dumpAllStats
			void registerUserMeasure(const char *event_name, const double value);

			/** Start of a named section \sa enter */
			inline void enter( const char *func_name ) {
				if (m_enabled)
					do_enter(func_name);
			}
			/** End of a named section \return The ellapsed time, in seconds or 0 if disabled. \sa enter */
			inline double leave( const char *func_name ) {
				return m_enabled ? do_leave(func_name) : 0;
			}
			/** Return the mean execution time of the given "section", or 0 if it hasn't ever been called "enter" with that section name */
			double getMeanTime(const std::string &name) const;
		}; // End of class def.


		/** A safe way to call enter() and leave() of a mrpt::utils::CTimeLogger upon construction and destruction of 
		 * this auxiliary object, making sure that leave() will be called upon exceptions, etc.
		 * Usage: 
		 * \code
		 *    CTimeLogger logger;
		 *    // ...
		 *    { // Start of scope to be monitorized
		 *       CTimeLoggerEntry tle(logger,"operation-name");
		 *
		 *       // do whatever
		 *
		 *    } // End of scope 
		 * \endcode
		 * \ingroup mrpt_base_grp
		 */
		struct BASE_IMPEXP CTimeLoggerEntry
		{
			CTimeLoggerEntry(CTimeLogger &logger, const char*section_name );
			~CTimeLoggerEntry();
			CTimeLogger &m_logger;
			const char *m_section_name;
		};


		/** @name Auxiliary stuff for the global profiler used in MRPT_START / MRPT_END macros.
		  @{ */
		extern CTimeLogger BASE_IMPEXP global_profiler;
		void BASE_IMPEXP global_profiler_enter(const char *func_name) MRPT_NO_THROWS;
		void BASE_IMPEXP global_profiler_leave(const char *func_name) MRPT_NO_THROWS;
		/** @} */

	} // End of namespace
} // End of namespace
#endif
