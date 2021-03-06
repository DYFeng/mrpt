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
#ifndef CPose3DPDFParticles_H
#define CPose3DPDFParticles_H

#include <mrpt/poses/CPose3DPDF.h>
#include <mrpt/bayes/CProbabilityParticle.h>
#include <mrpt/bayes/CParticleFilterCapable.h>
#include <mrpt/bayes/CParticleFilterData.h>

namespace mrpt
{
	namespace poses
	{
		using namespace mrpt::bayes;
		using namespace mrpt::utils;

		// This must be added to any CSerializable derived class:
		DEFINE_SERIALIZABLE_PRE_CUSTOM_BASE(CPose3DPDFParticles,CPose3DPDF)

		/** Declares a class that represents a Probability Density function (PDF) of a 3D pose
		 *
		 *  This class is also the base for the implementation of Monte-Carlo Localization (MCL), in mrpt::slam::CMonteCarloLocalization2D.
		 *
		 *  See the application "app/pf-localization" for an example of usage.
		 *
		 * \sa CPose3D, CPose3DPDF, CPoseGaussianPDF
		 * \ingroup poses_pdf_grp
		 */
		class BASE_IMPEXP CPose3DPDFParticles :
			public CPose3DPDF,
			public mrpt::bayes::CParticleFilterData<CPose3D>,
			public mrpt::bayes::CParticleFilterDataImpl<CPose3DPDFParticles,mrpt::bayes::CParticleFilterData<CPose3D>::CParticleList>
		{
			// This must be added to any CSerializable derived class:
			DEFINE_SERIALIZABLE( CPose3DPDFParticles )

		 public:
			/** Constructor
			  * \param M The number of m_particles.
			  */
			CPose3DPDFParticles( size_t M = 1 );

			/** Copy constructor:
			  */
			inline CPose3DPDFParticles( const CPose3DPDFParticles& obj ) :
				CPose3DPDF(),
				CParticleFilterData<CPose3D>()
			{
				copyFrom( obj );
			}

			/** Destructor
			 */
			virtual ~CPose3DPDFParticles();


			/** Copy operator, translating if necesary (for example, between m_particles and gaussian representations)
			  */
			void  copyFrom(const CPose3DPDF &o);

			/** Reset the PDF to a single point: All m_particles will be set exactly to the supplied pose.
			  * \param location The location to set all the m_particles.
			  * \param particlesCount If this is set to 0 the number of m_particles remains unchanged.
			  *  \sa resetUniform, resetUniformFreeSpace
			  */
			void  resetDeterministic( const CPose3D &location,
												size_t	particlesCount = 0);

			 /** Returns an estimate of the pose, (the mean, or mathematical expectation of the PDF), computed as a weighted average over all m_particles.
			   * \sa getCovariance
			   */
			void getMean(CPose3D &mean_pose) const;

			/** Returns an estimate of the pose covariance matrix (6x6 cov matrix) and the mean, both at once.
			  * \sa getMean
			  */
			void getCovarianceAndMean(CMatrixDouble66 &cov,CPose3D &mean_point) const;

			/** Returns the pose of the i'th particle.
			  */
			CPose3D	 getParticlePose(int i) const;

			/** Save PDF's m_particles to a text file. In each line it will go: "x y z"
			 */
			void  saveToTextFile(const std::string &file) const;

			/** Get the m_particles count (equivalent to "particlesCount")
			 */
			size_t  size() const { return m_particles.size(); }

			/** this = p (+) this. This can be used to convert a PDF from local coordinates to global, providing the point (newReferenceBase) from which
			  *   "to project" the current pdf. Result PDF substituted the currently stored one in the object.
			  */
			void  changeCoordinatesReference( const CPose3D &newReferenceBase );

			/** Draws a single sample from the distribution (WARNING: weights are assumed to be normalized!)
			  */
			void  drawSingleSample( CPose3D &outPart ) const;

			/** Draws a number of samples from the distribution, and saves as a list of 1x6 vectors, where each row contains a (x,y,phi) datum.
			  */
			void  drawManySamples( size_t N, std::vector<vector_double> & outSamples ) const;

			/** Appends (pose-composition) a given pose "p" to each particle
			  */
			void  operator += ( const CPose3D &Ap);

			/** Appends (add to the list) a set of m_particles to the existing ones, and then normalize weights.
			  */
			void append( CPose3DPDFParticles &o );

			/** Returns a new PDF such as: NEW_PDF = (0,0,0) - THIS_PDF
			  */
			void inverse(CPose3DPDF &o) const;

			/** Returns the particle with the highest weight.
			  */
			CPose3D	 getMostLikelyParticle() const;

			/** Bayesian fusion.
			  */
			void  bayesianFusion( const CPose3DPDF &p1, const CPose3DPDF &p2 );

		}; // End of class def.


	} // End of namespace
} // End of namespace

#endif
