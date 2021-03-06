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
#ifndef _MRPT_MONTE_CARLO_H_
#define _MRPT_MONTE_CARLO_H_

#include <map>
#include <cmath>
#include <vector>
#include <numeric>
#include <algorithm>
#include <stdexcept>
#include <functional>
//#include <mrpt/gui/CDisplayWindowPlots.h>
#include <mrpt/random.h>
#include <mrpt/utils/CTicTac.h>

namespace mrpt	{	namespace math	{

	/** Montecarlo simulation for experiments in 1D.
	     Template arguments are:
			- T: base type, i.e., if an experiment needs to generate random points, then T may be a TPoint3D, and so on.
			- NUM: the numeric type used to represent the error. Usually, double.
			- OTHER: an intermediate type, used especially when testing inverse functions. Leave as int or double if you don't use it.

		HOW TO USE THIS CLASS:
			- Create an instance of the class.
			- Refill the "valueGenerator" member with an appropriate function.
			- If your experiment calculates the error directly from the base value, then refill the "errorFun1" member.
			- Otherwise, if your experiment involves the calculation of some value whom with the experimental function is compared, refill "intermediateFun" and "errorFun2".
			- Refill only on of the alternatives.
	 * \ingroup mrpt_base_grp
	*/
	template<typename T,typename NUM,typename OTHER> class CMonteCarlo	{
	private:
		mrpt::random::CRandomGenerator gen;
		class CStatisticalAnalyzer	{
		private:
			Eigen::Matrix<NUM,Eigen::Dynamic,1> data;
		public:
			template<typename VEC> inline CStatisticalAnalyzer(const VEC &v1):data(v1.begin(),v1.end())	{}
			template<typename VEC> inline void setData(const VEC &v1)	{
				data.assign(v1.begin(),v1.end());
			}
			template<typename VEC> inline void getData(VEC &v1) const	{
				v1.assign(data.begin(),data.end());
			}
			template<typename VEC1,typename VEC2> inline void getDistribution(VEC1 &vx,VEC2 &vy,const NUM width=1.0) const	{
				mrpt::vector_double vvx,vvy;
				getDistribution(vvx,vvy,width);
				vx.assign(vvx.begin(),vvx.end());
				vy.assign(vvy.begin(),vvy.end());
			}
			// Function overload, not specialization (GCC complains otherwise):
			inline void getDistribution(mrpt::vector_double &vx,mrpt::vector_double &vy,const NUM width=1.0) const	{
				CHistogram hist(CHistogram::createWithFixedWidth(0,*max_element(data.begin(),data.end()),width));
				hist.add(data);
				hist.getHistogram(vx,vy);
			}

		};
	public:
		//TODO: use templates for function types.
		//Random generator.
		T (*valueGenerator)(mrpt::random::CRandomGenerator &);
		//Computes automatically the error (without an intermediate type)
		NUM (*errorFun1)(const T &);

		OTHER (*intermediateFun)(const T &);
		NUM (*errorFun2)(const T &,const OTHER &);
		inline CMonteCarlo():gen(),valueGenerator(NULL),errorFun1(NULL),intermediateFun(NULL),errorFun2(NULL)	{}
		NUM doExperiment(size_t N,double &time,bool showInWindow=false)	{
			if (!valueGenerator) throw std::logic_error("Value generator function is not set.");
			std::vector<T> baseData(N);
			std::vector<NUM> errorData(N);
			mrpt::utils::CTicTac meter;
			for (size_t i=0;i<N;++i) baseData[i]=valueGenerator(gen);
			if (errorFun1)	{
				meter.Tic();
				std::transform(baseData.begin(),baseData.end(),errorData.begin(),errorFun1);
				time=meter.Tac();
			}	else	{
				if (!intermediateFun||!errorFun2) throw std::logic_error("Experiment-related functions are not set.");
				std::vector<OTHER> intermediate(N);
				transform(baseData.begin(),baseData.end(),intermediate.begin(),intermediateFun);
				meter.Tic();
				for (size_t i=0;i<N;++i) errorData[i]=errorFun2(baseData[i],intermediate[i]);
				time=meter.Tac();
			}
			NUM res=accumulate(errorData.begin(),errorData.end(),NUM(0))/errorData.size();
			//if (showInWindow)	{
			//	CStatisticalAnalyzer st(errorData);
			//	mrpt::gui::CDisplayWindowPlots wnd("Error results from Monte Carlo simulation");
			//	std::vector<NUM> errorX,errorY;
			//	st.getDistribution(errorX,errorY,0.1);
			//	wnd.plot(errorX,errorY,"b-","Plot1");
			//	NUM maxVal=*std::max_element(errorY.begin(),errorY.end());
			//	std::vector<NUM> dx(2,res),dy(mrpt::utils::make_vector<2,NUM>(0,maxVal));
			//	wnd.plot(dx,dy,"r-","Plot2");
			//	while (wnd.isOpen());
			//}
			return res;
		}
	};

}}	//End of namespaces
#endif
