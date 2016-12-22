// *************************************************************************************************
// Horde3D Shader Utility Library
// --------------------------------------
//		- Skinning functions -
//
// Copyright (C) 2006-2011 Nicolas Schulz
//	D3D11 version 2012-2013 Attila Kocsis
//
// You may use the following code in projects based on the Horde3D graphics engine.
//
// *************************************************************************************************

float4 skinMatRows[75*3];


float4x4 getJointMat( const int jointIndex )
{
	// Note: This matrix is transposed so vec/mat multiplications need to be done in reversed order
	return float4x4( skinMatRows[jointIndex * 3],
				 skinMatRows[jointIndex * 3 + 1],
				 skinMatRows[jointIndex * 3 + 2],
				 float4( 0, 0, 0, 1 ) );
}

float4x4 calcSkinningMat(float4 weights, float4 joints)
{
	return weights.x * getJointMat( int( joints.x) ) +
		   weights.y * getJointMat( int( joints.y) ) +
		   weights.z * getJointMat( int( joints.z) ) +
		   weights.w * getJointMat( int( joints.w) );
}

float3x3 getSkinningMatVec( const float4x4 skinningMat )
{
	return float3x3( skinningMat[0].xyz, skinningMat[1].xyz, skinningMat[2].xyz );
}

float4 skinPos( const float4 pos, float4 weights, float4 joints )
{
	return mul( getJointMat( int( joints.x ) ), pos ) * weights.x +
		   mul( getJointMat( int( joints.y ) ), pos ) * weights.y +
		   mul( getJointMat( int( joints.z ) ), pos ) * weights.z +
		   mul( getJointMat( int( joints.w ) ), pos ) * weights.w;
}

float4 skinPos( const float4 pos, const float4x4 skinningMat )
{
//	return mul(pos, skinningMat);	--need to reverse
	return mul(skinningMat, pos);
}

float3 skinVec( const float3 vec, const float3x3 skinningMatTSB )
{
	return mul(vec, skinningMatTSB);
}
