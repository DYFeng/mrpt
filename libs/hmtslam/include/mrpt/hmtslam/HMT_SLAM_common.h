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
#ifndef HMT_SLAM_common_H
#define HMT_SLAM_common_H

#include <mrpt/utils/CSerializable.h>
#include <mrpt/utils/stl_extensions.h>

#include <mrpt/hmtslam/link_pragmas.h>


#define COMMON_TOPOLOG_HYP					static_cast<THypothesisID>(0)

#define NODE_ANNOTATION_METRIC_MAPS 		"metricMaps"         // CMultiMetricMap
#define NODE_ANNOTATION_REF_POSEID 			"refPoseID"			 // TPoseID
#define NODE_ANNOTATION_POSES_GRAPH 		"posesGraph"         // CRobotPosesGraph

#define NODE_ANNOTATION_PLACE_POSE 			"placePose"          // CPoint2D

#define ARC_ANNOTATION_DELTA				"Delta"				 // CPose3DPDF (for the current implementation, it's a CPose3DPDFParticles)
#define ARC_ANNOTATION_DELTA_SRC_POSEID		"Delta_poseID_src"	 // TPoseID (elemental datatype)
#define ARC_ANNOTATION_DELTA_TRG_POSEID		"Delta_poseID_trg"	 // TPoseID (elemental datatype)

/**  Used in constructor of mrpt::hmtslam::CHMHMapArc */
#define ARC_TYPES			"Membership,Navegability,RelativePose,Location"
#define DEFAULT_ARC_TYPE	"Membership"

/**  Used in constructor of mrpt::hmtslam::CHMHMapNode */
#define NODE_TYPES			"Place,Area,TopologicalMap,Object"
#define DEFAULT_NODE_TYPE	"Place"

// Used as current robot pose when initializing an empty HMT-map.
#define POSEID_INVALID	static_cast<TPoseID>(-1)

#define AREAID_INVALID	static_cast<uint64_t>(-1)

#define MSG_SOURCE_LSLAM    1
#define MSG_SOURCE_AA       2

namespace mrpt
{
	namespace hmtslam
	{
		using namespace mrpt::utils;

		class HMTSLAM_IMPEXP CHMHMapArc;
		class HMTSLAM_IMPEXP CHMHMapNode;

		DEFINE_SERIALIZABLE_PRE_CUSTOM_BASE_LINKAGE( CHMHMapArc,mrpt::utils::CSerializable, HMTSLAM_IMPEXP )


		/** An integer number uniquely identifying each of the concurrent hypotheses for the robot topological path (& possibly local metric clusters) in HMT-SLAM.
		  *   The number 0 has the special meaning of "that part of the map/robot path in which all hypotheses agree".
		  *  They can be generated from CHMTSLAM::generateHypothesisID()
		  */
		typedef int64_t  THypothesisID;

		/** An integer number uniquely identifying each robot pose stored in HMT-SLAM.
		  * They can be generated from CHMTSLAM::generatePoseID()
		  */
		typedef uint64_t  TPoseID;

		typedef std::pair<TPoseID,TPoseID>  TPairPoseIDs;

		typedef std::vector<TPoseID> TPoseIDList;
		typedef std::set<TPoseID> TPoseIDSet;

		DEFINE_SERIALIZABLE_PRE_CUSTOM_BASE_LINKAGE( THypothesisIDSet,mrpt::utils::CSerializable, HMTSLAM_IMPEXP )

		/** A set of hypothesis IDs, used for arcs and nodes in multi-hypothesis hybrid maps.
		  *  \sa THypothesisID, CHierarchicalMHMap
		  * \ingroup mrpt_hmtslam_grp
		  */
		class HMTSLAM_IMPEXP THypothesisIDSet : public mrpt::utils::CSerializable, public std::set<THypothesisID>
		{
			// This must be added to any CSerializable derived class:
			DEFINE_SERIALIZABLE( THypothesisIDSet )

		public:
			/** Default constructor
			  */
			THypothesisIDSet()
			{
			}

			/** Constructor with one initial element
			  */
			THypothesisIDSet( const THypothesisID& val)
			{
				insert( val );
			}

			virtual ~THypothesisIDSet()
			{
			}

			/** Returns true if the hypothesis is into the set.
			  */
			bool has(  const THypothesisID& val ) const
			{
				return find(val)!=end() || find(COMMON_TOPOLOG_HYP)!=end();
			}

			/** Dump to console.
			  */
			void debugDump() const;

		};


		/** A class for storing a sequence of arcs (a path).
		  * \sa CHMTSLAM
		  */
		class HMTSLAM_IMPEXP TArcList : public list_searchable<mrpt::hmtslam::CHMHMapArcPtr>
		{
		private:
			typedef list_searchable<mrpt::hmtslam::CHMHMapArcPtr> BASE;

		public:
			void  debugDump();
			void read( utils::CStream &in );
			void write( utils::CStream &out ) const;

		};


	} // End of namespace
} // End of namespace



#endif
