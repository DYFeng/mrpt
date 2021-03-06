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
#ifndef CMetricMapBuilderICP_H
#define CMetricMapBuilderICP_H

#include <mrpt/slam/CMetricMapBuilder.h>
#include <mrpt/slam/CICP.h>
#include <mrpt/poses/CRobot2DPoseEstimator.h>

#include <mrpt/slam/link_pragmas.h>


namespace mrpt
{
namespace slam
{
	/** A class for very simple 2D SLAM based on ICP. This is a non-probabilistic pose tracking algorithm.
	 *   Map are stored as in files as binary dumps of "mrpt::slam::CSimpleMap" objects. The methods are
	 *	 thread-safe.
	 * \ingroup metric_slam_grp
	 */
	class SLAM_IMPEXP  CMetricMapBuilderICP : public CMetricMapBuilder
	{
	 public:
		 /** Default constructor - Upon construction, you can set the parameters in ICP_options, then call "initialize".
		   */
		 CMetricMapBuilderICP();

		 /** Destructor:
		   */
		 virtual ~CMetricMapBuilderICP();

		 /** Algorithm configuration params
		   */
		 struct SLAM_IMPEXP TConfigParams : public mrpt::utils::CLoadableOptions
		 {
			 /** Initializer */
			 TConfigParams ();
			 virtual void  loadFromConfigFile(
				const mrpt::utils::CConfigFileBase	&source,
				const std::string		&section);
			 virtual void  dumpToTextStream( CStream		&out) const;

			/** (default:false) Match against the occupancy grid or the points map? The former is quicker but less precise. */
			bool	matchAgainstTheGrid;

			double insertionLinDistance;	//!< Minimum robot linear (m) displacement for a new observation to be inserted in the map.
			double insertionAngDistance;	//!< Minimum robot angular (rad, deg when loaded from the .ini) displacement for a new observation to be inserted in the map.
			double localizationLinDistance;	//!< Minimum robot linear (m) displacement for a new observation to be used to do ICP-based localization (otherwise, dead-reckon with odometry).
			double localizationAngDistance;//!< Minimum robot angular (rad, deg when loaded from the .ini) displacement for a new observation to be used to do ICP-based localization (otherwise, dead-reckon with odometry).

			double minICPgoodnessToAccept;  //!< Minimum ICP goodness (0,1) to accept the resulting corrected position (default: 0.40)

			/** What maps to create (at least one points map and/or a grid map are needed).
			  *  For the expected format in the .ini file when loaded with loadFromConfigFile(), see documentation of TSetOfMetricMapInitializers.
			  */
			TSetOfMetricMapInitializers	mapInitializers;

		 };

		 TConfigParams			ICP_options; //!< Options for the ICP-SLAM application \sa ICP_params
		 CICP::TConfigParams	ICP_params;  //!< Options for the ICP algorithm itself \sa ICP_options

		/** Initialize the method, starting with a known location PDF "x0"(if supplied, set to NULL to left unmodified) and a given fixed, past map.
		  *  This method MUST be called if using the default constructor, after loading the configuration into ICP_options. In particular, TConfigParams::mapInitializers
		  */
		void  initialize(
			const CSimpleMap &initialMap  = CSimpleMap(),
			CPosePDF					*x0 = NULL
			);

		/** Returns a copy of the current best pose estimation as a pose PDF.
		  */
		CPose3DPDFPtr  getCurrentPoseEstimation() const;

		 /** Sets the "current map file", thus that map will be loaded if it exists or a new one will be created if it does not, and the updated map will be save to that file when destroying the object.
		   */
		 void  setCurrentMapFile( const char *mapFile );

		/** Appends a new action and observations to update this map: See the description of the class at the top of this page to see a more complete description.
		 *  \param action The estimation of the incremental pose change in the robot pose.
		 *	\param in_SF The set of observations that robot senses at the new pose.
		 * See params in CMetricMapBuilder::options and CMetricMapBuilderICP::ICP_options
		 * \sa processObservation
		 */
		void  processActionObservation(
					CActionCollection	&action,
					CSensoryFrame		&in_SF );

		/**  The main method of this class: Process one odometry or sensor observation.
		    The new entry point of the algorithm (the old one  was processActionObservation, which now is a wrapper to
		  this method).
		 * See params in CMetricMapBuilder::options and CMetricMapBuilderICP::ICP_options
		  */
		void  processObservation(const CObservationPtr &obs);


		/** Fills "out_map" with the set of "poses"-"sensory-frames", thus the so far built map.
		  */
		void  getCurrentlyBuiltMap(CSimpleMap &out_map) const;


		 /** Returns the 2D points of current local map
		   */
		void  getCurrentMapPoints( std::vector<float> &x, std::vector<float> &y);

		/** Returns the map built so far. NOTE that for efficiency a pointer to the internal object is passed, DO NOT delete nor modify the object in any way, if desired, make a copy of ir with "duplicate()".
		  */
		CMultiMetricMap*   getCurrentlyBuiltMetricMap();

		/** Returns just how many sensory-frames are stored in the currently build map.
		  */
		unsigned int  getCurrentlyBuiltMapSize();

		/** A useful method for debugging: the current map (and/or poses) estimation is dumped to an image file.
		  * \param file The output file name
		  * \param formatEMF_BMP Output format = true:EMF, false:BMP
		  */
		void  saveCurrentEstimationToImage(const std::string &file, bool formatEMF_BMP = true);

	 private:
		 /** The set of observations that leads to current map:
		   */
		 CSimpleMap		SF_Poses_seq;

		 /** The metric map representation as a points map:
		   */
		 CMultiMetricMap			metricMap;

		 /** Current map file.
		   */
		 std::string				currentMapFile;

		 /** The pose estimation by the alignment algorithm (ICP). */
		 mrpt::poses::CRobot2DPoseEstimator		m_lastPoseEst;  //!< Last pose estimation (Mean)
		 mrpt::math::CMatrixDouble33			m_lastPoseEst_cov; //!< Last pose estimation (covariance)

		 /** The estimated robot path:
		   */
		 std::deque<mrpt::math::TPose2D>		m_estRobotPath;
		 mrpt::poses::CPose2D					m_auxAccumOdometry;

		/** Traveled distances from last map update / ICP-based localization. */
		struct SLAM_IMPEXP TDist
		{
			TDist() : lin(0),ang(0) { }
			double lin; // meters
			double ang; // degrees
			mrpt::poses::CPose2D  last_update;
			
			void updateDistances(const mrpt::poses::CPose2D &p);
			void updatePose(const mrpt::poses::CPose2D &p);
		};
		TDist    m_distSinceLastICP;
		mrpt::aligned_containers<std::string,TDist>::map_t  m_distSinceLastInsertion; //!< Indexed by sensor label.
		bool	 m_there_has_been_an_odometry;

		void accumulateRobotDisplacementCounters(const CPose2D & new_pose);
		void resetRobotDisplacementCounters(const CPose2D & new_pose);

	};

	} // End of namespace
} // End of namespace

#endif
