/*
	BlamLib: .NET SDK for the Blam Engine

	See license\BlamLib\BlamLib for specific license information
*/

#include <boost/preprocessor/iteration/iterate.hpp>

#if !BOOST_PP_IS_ITERATING

	#if defined(__XNA_MATH_IMPL_CODE_INCLUDE_ID)
		#define BOOST_PP_ITERATION_PARAMS_1 (3, (__XNA_MATH_IMPL_CODE_INCLUDE_ID, __XNA_MATH_IMPL_CODE_INCLUDE_ID, "XNA/MathImpl.UShort4N.inl"))
		??=include BOOST_PP_ITERATE()
		#undef __XNA_MATH_IMPL_CODE_INCLUDE_ID
	#endif


	//////////////////////////////////////////////////////////////////////////
	// C++/clr header code
	#elif BOOST_PP_ITERATION() == __XNA_MATH_IMPL_CODE_CLR_HPP

		static LowLevel::Math::real_quaternion ConvertUShort4N(mcpp_ushort value_x, mcpp_ushort value_y, mcpp_ushort value_z, mcpp_ushort value_w);

		static void ConvertUShort4N(LowLevel::Math::real_quaternion input, 
			mcpp_out(mcpp_ushort) value_x, mcpp_out(mcpp_ushort) value_y, mcpp_out(mcpp_ushort) value_z, mcpp_out(mcpp_ushort) value_w);



	//////////////////////////////////////////////////////////////////////////
	// C++/clr source code
	#elif BOOST_PP_ITERATION() == __XNA_MATH_IMPL_CODE_CLR_CPP

		LowLevel::Math::real_quaternion Math::ConvertUShort4N(mcpp_ushort value_x, mcpp_ushort value_y, mcpp_ushort value_z, mcpp_ushort value_w)
		{
			LowLevel::Math::real_quaternion ret = LowLevel::Math::real_quaternion();

			pin_ptr<mcpp_real> output = &ret.Vector.I;
			::Internal::ConvertUShort4N(value_x, value_y, value_z, value_w, output);

			return ret;
		}

		void Math::ConvertUShort4N(LowLevel::Math::real_quaternion input, 
			mcpp_out(mcpp_ushort) value_x, mcpp_out(mcpp_ushort) value_y, mcpp_out(mcpp_ushort) value_z, mcpp_out(mcpp_ushort) value_w)
		{
			pin_ptr<mcpp_real> vec = &input.Vector.I;
			mcpp_ushort v_x, v_y, v_z, v_w;
			::Internal::StoreUShort4N(v_x, v_y, v_z, v_w, vec);
			value_x = v_x;	value_y = v_y;	value_z = v_z;	value_w = v_w;
		}



	//////////////////////////////////////////////////////////////////////////
	// Native C++ code
	#elif BOOST_PP_ITERATION() == __XNA_MATH_IMPL_CODE_CPP

		static void ConvertUShort4N(cpp_ushort value_x, cpp_ushort value_y, cpp_ushort value_z, cpp_ushort value_w, cpp_real* output)
		{
			XMUSHORTN4 xm_type; xm_type.x = value_x; xm_type.y = value_y; xm_type.z = value_z; xm_type.w = value_w;

			XMVECTOR vec = ::XMLoadUShortN4(&xm_type);

#if defined(_XM_SSE_INTRINSICS_) && !defined(_XM_NO_INTRINSICS_)
			_mm_storeu_ps(output, vec);
#else
			output[0] = vec.v[0];	output[1] = vec.v[1];	output[2] = vec.v[2];
			output[3] = vec.v[3];
#endif
		}

		static void StoreUShort4N(cpp_ushort& value_x, cpp_ushort& value_y, cpp_ushort& value_z, cpp_ushort& value_w, cpp_real* input)
		{
			XMVECTOR vec;
#if defined(_XM_SSE_INTRINSICS_) && !defined(_XM_NO_INTRINSICS_)
			vec = _mm_set_ps(input[0], input[1], input[2], input[3]);
#else
			vec.v[0] = input[0];	vec.v[1] = input[1];	vec.v[2] = input[2];
			vec.v[3] = input[3];
#endif

			XMUSHORTN4 xm_type;
			::XMStoreUShortN4(&xm_type, vec);
			value_x = xm_type.x;	value_y = xm_type.y;	value_z = xm_type.z;	value_w = xm_type.w;
		}

#else
#endif