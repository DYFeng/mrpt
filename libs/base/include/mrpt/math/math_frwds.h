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
#ifndef mrpt_math_forwddecls_H
#define mrpt_math_forwddecls_H

#include <mrpt/config.h>
#include <mrpt/base/link_pragmas.h>
#include <cmath>
#include <cstdlib>
#include <algorithm>
#include <string>

/*! \file math_frwds.h
  * Forward declarations of all mrpt::math classes related to vectors, arrays and matrices.
  * Many of the function implementations are in ops_matrices.h, others in ops_containers.h
  */

namespace mrpt
{
	namespace utils
	{
		class BASE_IMPEXP CStream;
		template<class T> inline T square(const T x);
	}

	namespace system
	{
		std::string BASE_IMPEXP MRPT_getVersion();
	}

	namespace poses
	{
		class CPoint2D;
		class CPoint3D;
		class CPose2D;
		class CPose3D;
		class CPose3DQuat;
	}

	namespace math
	{
		struct TPoint2D;
		struct TPoint3D;
		struct TPose2D;
		struct TPose3D;
		struct TPose3DQuat;

		namespace detail
		{
			/** Internal resize which compiles to nothing on fixed-size matrices. */
			template <typename MAT,int TypeSizeAtCompileTime>
			struct TAuxResizer {
				static inline void internal_resize(MAT &obj, size_t row, size_t col) { }
				static inline void internal_resize(MAT &obj, size_t nsize) { }
			};
			template <typename MAT>
			struct TAuxResizer<MAT,-1> {
				static inline void internal_resize(MAT &obj, size_t row, size_t col) { obj.derived().conservativeResize(row,col); }
				static inline void internal_resize(MAT &obj, size_t nsize) { obj.derived().conservativeResize(nsize); }
			};
		}


		/*! Selection of the number format in CMatrixTemplate::saveToTextFile
		  */
		enum TMatrixTextFileFormat
		{
			MATRIX_FORMAT_ENG = 0,   //!< engineering format '%e'
			MATRIX_FORMAT_FIXED = 1, //!< fixed floating point '%f'
			MATRIX_FORMAT_INT = 2	 //!< intergers '%i'
		};

		/** For usage in one of the constructors of CMatrixFixedNumeric or CMatrixTemplate (and derived classes), if it's not required
		     to fill it with zeros at the constructor to save time. */
		enum TConstructorFlags_Matrices
		{
			UNINITIALIZED_MATRIX = 0
		};

		// ---------------- Forward declarations: Classes ----------------
		template <class T> class CMatrixTemplate;
		template <class T> class CMatrixTemplateObjects;

#define MRPT_MATRIX_CONSTRUCTORS_FROM_POSES(_CLASS_) \
		explicit inline _CLASS_( const mrpt::math::TPose2D &p)  { mrpt::math::containerFromPoseOrPoint(*this,p); } \
		explicit inline _CLASS_( const mrpt::math::TPose3D &p)  { mrpt::math::containerFromPoseOrPoint(*this,p); } \
		explicit inline _CLASS_( const mrpt::math::TPose3DQuat &p)  { mrpt::math::containerFromPoseOrPoint(*this,p); } \
		explicit inline _CLASS_( const mrpt::math::TPoint2D &p) { mrpt::math::containerFromPoseOrPoint(*this,p); } \
		explicit inline _CLASS_( const mrpt::math::TPoint3D &p) { mrpt::math::containerFromPoseOrPoint(*this,p); } \
		explicit inline _CLASS_( const mrpt::poses::CPose2D &p)  { mrpt::math::containerFromPoseOrPoint(*this,p); } \
		explicit inline _CLASS_( const mrpt::poses::CPose3D &p)  { mrpt::math::containerFromPoseOrPoint(*this,p); } \
		explicit inline _CLASS_( const mrpt::poses::CPose3DQuat &p)  { mrpt::math::containerFromPoseOrPoint(*this,p); } \
		explicit inline _CLASS_( const mrpt::poses::CPoint2D &p) { mrpt::math::containerFromPoseOrPoint(*this,p); } \
		explicit inline _CLASS_( const mrpt::poses::CPoint3D &p) { mrpt::math::containerFromPoseOrPoint(*this,p); }


		template <class CONTAINER1,class CONTAINER2> void cumsum(const CONTAINER1 &in_data, CONTAINER2 &out_cumsum);

		template <class CONTAINER> inline typename CONTAINER::Scalar norm(const CONTAINER &v);
		template <class CONTAINER> inline typename CONTAINER::Scalar norm_inf(const CONTAINER &v);

		template <class MAT_A,class SKEW_3VECTOR,class MAT_OUT> void multiply_A_skew3(const MAT_A &A,const SKEW_3VECTOR &v, MAT_OUT &out);
		template <class SKEW_3VECTOR,class MAT_A,class MAT_OUT> void multiply_skew3_A(const SKEW_3VECTOR &v,const MAT_A &A, MAT_OUT &out);

		namespace detail
		{
			// Implemented in "lightweight_geom_data.cpp"
			TPoint2D BASE_IMPEXP lightFromPose(const mrpt::poses::CPoint2D &p);	//!< Convert a pose into a light-weight structure (functional form, needed for forward declarations)
			TPoint3D BASE_IMPEXP lightFromPose(const mrpt::poses::CPoint3D &p);	//!< Convert a pose into a light-weight structure (functional form, needed for forward declarations)
			TPose2D  BASE_IMPEXP lightFromPose(const mrpt::poses::CPose2D &p);	//!< Convert a pose into a light-weight structure (functional form, needed for forward declarations)
			TPose3D  BASE_IMPEXP lightFromPose(const mrpt::poses::CPose3D &p);	//!< Convert a pose into a light-weight structure (functional form, needed for forward declarations)
			TPose3DQuat BASE_IMPEXP lightFromPose(const mrpt::poses::CPose3DQuat &p);	//!< Convert a pose into a light-weight structure (functional form, needed for forward declarations)

			template <class MATORG, class MATDEST>
			void extractMatrix(
				const MATORG &M,
				const size_t first_row,
				const size_t first_col,
				MATDEST &outMat);
		}

		/** Conversion of poses to MRPT containers (vector/matrix) */
		template <class CONTAINER> CONTAINER & containerFromPoseOrPoint(CONTAINER &C, const TPoint2D &p);
		template <class CONTAINER> CONTAINER & containerFromPoseOrPoint(CONTAINER &C, const TPoint3D &p);
		template <class CONTAINER> CONTAINER & containerFromPoseOrPoint(CONTAINER &C, const TPose2D &p);
		template <class CONTAINER> CONTAINER & containerFromPoseOrPoint(CONTAINER &C, const TPose3D &p);
		template <class CONTAINER> CONTAINER & containerFromPoseOrPoint(CONTAINER &C, const TPose3DQuat &p);

		template <class CONTAINER> inline CONTAINER & containerFromPoseOrPoint(CONTAINER &C, const mrpt::poses::CPoint2D &p) { return containerFromPoseOrPoint(C, mrpt::math::detail::lightFromPose(p)); }
		template <class CONTAINER> inline CONTAINER & containerFromPoseOrPoint(CONTAINER &C, const mrpt::poses::CPoint3D &p) { return containerFromPoseOrPoint(C, mrpt::math::detail::lightFromPose(p)); }
		template <class CONTAINER> inline CONTAINER & containerFromPoseOrPoint(CONTAINER &C, const mrpt::poses::CPose2D &p)  { return containerFromPoseOrPoint(C, mrpt::math::detail::lightFromPose(p)); }
		template <class CONTAINER> inline CONTAINER & containerFromPoseOrPoint(CONTAINER &C, const mrpt::poses::CPose3D &p)  { return containerFromPoseOrPoint(C, mrpt::math::detail::lightFromPose(p)); }
		template <class CONTAINER> inline CONTAINER & containerFromPoseOrPoint(CONTAINER &C, const mrpt::poses::CPose3DQuat &p)  { return containerFromPoseOrPoint(C, mrpt::math::detail::lightFromPose(p)); }


		// Vicinity classes ----------------------------------------------------
		namespace detail	{
			/**
			  * The purpose of this class is to model traits for containers, so that they can be used as return values for the function CMatrixTemplate::getVicinity.
			  * This class is NOT defined for any base container, because correctness would not be guaranteed. Instead, each class must define its own specialization
			  * of the template, containing two functions:
			  * - static void initialize(container<T>,size_t N): must reserve space to allow at least the insertion of N*N elements, in a square fashion when appliable.
			  * - static void insertInContainer(container<T>,size_t r,size_t c,const T &): must insert the given element in the container. Whenever it's possible, it
			  * must insert it in the (r,c) coordinates.
			  * For linear containers, the vicinity functions are guaranteed to insert elements in order, i.e., starting from the top and reading from left to right.
			  */
			template<typename T> class VicinityTraits;

			/**
			  * This huge template encapsulates a function to get the vicinity of an element, with maximum genericity. Although it's not meant to be called directly,
			  * every type defining the ASSERT_ENOUGHROOM assert and the get_unsafe method will work. The assert checks if the boundaries (r-N,r+N,c-N,c+N) fit in
			  * the matrix.
			  * The template parameters are the following:
			  * - MatrixType: the matrix or container base type, from which the vicinity is required.
			  * - T: the base type of the matrix or container.
			  * - ReturnType: the returning container type. The class VicinityTraits<ReturnType> must be completely defined.
			  * - D: the dimension of the vicinity. Current implementations are 4, 5, 8, 9, 12, 13, 20, 21, 24 and 25, although it's easy to implement new variants.
			  */
			template<typename MatrixType,typename T,typename ReturnType,size_t D> struct getVicinity;

		}

		// Other forward decls:
		template <class T> T wrapTo2Pi(T a);


	} // End of namespace
} // End of namespace

#endif
