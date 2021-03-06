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


#include <mrpt/poses/CPoint2DPDFGaussian.h>
#include <mrpt/poses/CPose3D.h>
#include <mrpt/poses/CPoint3D.h>
#include <mrpt/math/CMatrixD.h>
#include <mrpt/random/RandomGenerators.h>

using namespace mrpt::poses;
using namespace mrpt::utils;
using namespace mrpt::random;

IMPLEMENTS_SERIALIZABLE( CPoint2DPDFGaussian, CPoint2DPDF, mrpt::poses )


/*---------------------------------------------------------------
	Constructor
  ---------------------------------------------------------------*/
CPoint2DPDFGaussian::CPoint2DPDFGaussian() : mean(0,0), cov()
{
}

/*---------------------------------------------------------------
	Constructor
  ---------------------------------------------------------------*/
CPoint2DPDFGaussian::CPoint2DPDFGaussian(
	const CPoint2D	&init_Mean,
	const CMatrixDouble22 &init_Cov ) : mean(init_Mean), cov(init_Cov)
{
}

/*---------------------------------------------------------------
	Constructor
  ---------------------------------------------------------------*/
CPoint2DPDFGaussian::CPoint2DPDFGaussian(
	const CPoint2D	&init_Mean ) : mean(init_Mean), cov()
{
}


/*---------------------------------------------------------------
						writeToStream
  ---------------------------------------------------------------*/
void  CPoint2DPDFGaussian::writeToStream(CStream &out,int *version) const
{
	if (version)
		*version = 0;
	else
	{
		out << CPoint2D(mean) << cov;
	}
}

/*---------------------------------------------------------------
						readFromStream
  ---------------------------------------------------------------*/
void  CPoint2DPDFGaussian::readFromStream(CStream &in,int version)
{
	switch(version)
	{
	case 0:
		{
			in >> mean >> cov;
		} break;
	default:
		MRPT_THROW_UNKNOWN_SERIALIZATION_VERSION(version)

	};
}


/*---------------------------------------------------------------
						operator =
  ---------------------------------------------------------------*/
void  CPoint2DPDFGaussian::copyFrom(const CPoint2DPDF &o)
{
	if (this == &o) return;		// It may be used sometimes

	// Convert to gaussian pdf:
	o.getCovarianceAndMean(cov,mean);
}

/*---------------------------------------------------------------

  ---------------------------------------------------------------*/
void  CPoint2DPDFGaussian::saveToTextFile(const std::string &file) const
{
	MRPT_START

	FILE	*f=os::fopen(file.c_str(),"wt");
	if (!f) return;

	os::fprintf(f,"%f %f\n", mean.x(), mean.y() );

	os::fprintf(f,"%f %f\n", cov(0,0),cov(0,1) );
	os::fprintf(f,"%f %f\n", cov(1,0),cov(1,1) );

	os::fclose(f);

	MRPT_END
}

/*---------------------------------------------------------------
						changeCoordinatesReference
 ---------------------------------------------------------------*/
void  CPoint2DPDFGaussian::changeCoordinatesReference(const CPose3D &newReferenceBase )
{
	// Clip the 3x3 rotation matrix
	const CMatrixDouble22 M = newReferenceBase.getRotationMatrix().block(0,0,2,2);

	// The mean:
	mean = CPoint2D( newReferenceBase + mean );

	// The covariance:
	cov = M*cov*M.transpose();
}

/*---------------------------------------------------------------
					bayesianFusion
 ---------------------------------------------------------------*/
void  CPoint2DPDFGaussian::bayesianFusion( const CPoint2DPDFGaussian &p1, const CPoint2DPDFGaussian &p2 )
{
	MRPT_START

	CMatrixDouble22	C1_inv;
	p1.cov.inv(C1_inv);

	CMatrixDouble22	C2_inv;
	p2.cov.inv(C2_inv);

	CMatrixDouble22	 L = C1_inv;
	L+= C2_inv;

	L.inv(cov); // The new cov.

	CMatrixDouble21	x1 = CMatrixDouble21(p1.mean);
	CMatrixDouble21	x2 = CMatrixDouble21(p2.mean);
	CMatrixDouble21 x = cov * ( C1_inv*x1 + C2_inv*x2 );

	mean.x( x.get_unsafe(0,0) );
	mean.y( x.get_unsafe(1,0) );

	std::cout << "IN1: " << p1.mean << "\n" << p1.cov << "\n";
	std::cout << "IN2: " << p2.mean << "\n" << p2.cov << "\n";
	std::cout << "OUT: " << mean << "\n" << cov << "\n";

	MRPT_END
}

/*---------------------------------------------------------------
					productIntegralWith
 ---------------------------------------------------------------*/
double  CPoint2DPDFGaussian::productIntegralWith( const CPoint2DPDFGaussian &p) const
{
	MRPT_START
	// --------------------------------------------------------------
	// 12/APR/2009 - Jose Luis Blanco:
	//  The integral over all the variable space of the product of two
	//   Gaussians variables amounts to simply the evaluation of
	//   a normal PDF at (0,0), with mean=M1-M2 and COV=COV1+COV2
	// ---------------------------------------------------------------
	CMatrixDouble22 C = cov + p.cov;	// Sum of covs:

	CMatrixDouble22 C_inv;
	C.inv(C_inv);

	CMatrixDouble21	MU(UNINITIALIZED_MATRIX); // Diff. of means
	MU.get_unsafe(0,0) = mean.x() - p.mean.x();
	MU.get_unsafe(1,0) = mean.y() - p.mean.y();

	return std::pow( M_2PI, -0.5*state_length )
		* (1.0/std::sqrt( C.det() ))
		* exp( -0.5* MU.multiply_HtCH_scalar(C_inv) );

	MRPT_END
}


/*---------------------------------------------------------------
					productIntegralNormalizedWith
 ---------------------------------------------------------------*/
double  CPoint2DPDFGaussian::productIntegralNormalizedWith( const CPoint2DPDFGaussian &p) const
{
	return std::exp( -0.5*square(mahalanobisDistanceTo(p)) );
}

/*---------------------------------------------------------------
					drawSingleSample
 ---------------------------------------------------------------*/
void CPoint2DPDFGaussian::drawSingleSample(CPoint2D &outSample) const
{
	MRPT_START

	// Eigen3 emits an out-of-array warning here, but it seems to be a false warning? (WTF)
	vector_double vec;
	randomGenerator.drawGaussianMultivariate(vec,cov);

	ASSERT_(vec.size()==2);
	outSample.x( mean.x() + vec[0] );
	outSample.y( mean.y() + vec[1] );

	MRPT_END
}

/*---------------------------------------------------------------
					bayesianFusion
 ---------------------------------------------------------------*/
void  CPoint2DPDFGaussian::bayesianFusion( const CPoint2DPDF &p1_, const CPoint2DPDF &p2_,const double &minMahalanobisDistToDrop )
{
	MRPT_START

	// p1: CPoint2DPDFGaussian, p2: CPosePDFGaussian:
	ASSERT_( p1_.GetRuntimeClass() == CLASS_ID(CPoint2DPDFGaussian) );
	ASSERT_( p2_.GetRuntimeClass() == CLASS_ID(CPoint2DPDFGaussian) );

	THROW_EXCEPTION("TODO!!!");

	MRPT_END
}

/*---------------------------------------------------------------
					mahalanobisDistanceTo
 ---------------------------------------------------------------*/
double CPoint2DPDFGaussian::mahalanobisDistanceTo( const CPoint2DPDFGaussian & other ) const
{
	// The difference in means:
	Eigen::Matrix<double,2,1> deltaX;
	deltaX[0] = other.mean.x() - mean.x();
	deltaX[1] = other.mean.y() - mean.y();

	// The inverse of the combined covs:
	return std::sqrt( deltaX.multiply_HtCH_scalar( (other.cov + this->cov).inverse() ) );
}

/** Returns the Mahalanobis distance from this PDF to some point */
double CPoint2DPDFGaussian::mahalanobisDistanceToPoint( const double x, const double y ) const
{
	// The difference in means:
	Eigen::Matrix<double,2,1> deltaX;
	deltaX[0] = x - mean.x();
	deltaX[1] = y - mean.y();

	// The inverse of the combined covs:
	return std::sqrt( deltaX.multiply_HtCH_scalar( this->cov.inverse() ) );
}

