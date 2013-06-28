uniform sampler2D u_Texture0;
uniform sampler2D u_Texture1;
uniform sampler2D u_Texture2;
uniform sampler2D u_Texture3;
uniform float u_Time;

uniform vec2 u_Resolution;
uniform float u_Zoom;
uniform vec2 u_MouseCoords;
//uniform vec4 u_SourceRect;
//uniform vec4 u_DestRect;


varying vec2 v_TexCoords;
varying vec2 v_Position;
varying vec4 v_Data0;
varying vec2 v_MouseCoords;

$

attribute vec4 a_Data0;


$

void main()	{

	vec2 a_TexCoords = uv.xy;
	v_TexCoords = uv.xy;
	v_Data0 = a_Data0;

	float zoomi = 1.0/u_Zoom;

	v_Position.x = position.x*u_Zoom/u_Resolution.x;
	v_Position.y = position.y*u_Zoom/u_Resolution.y;

	v_MouseCoords.x = u_MouseCoords.x / u_Resolution.x;
	v_MouseCoords.y = u_MouseCoords.y / u_Resolution.y;

	gl_Position = vec4( -1.0 + (position.x*u_Zoom/u_Resolution.x)*2.0, 1.0 - (position.y*u_Zoom/u_Resolution.y)*2.0, position.z, 1.0 );

}

$

void main()	{

	/*
	//uniforms
	vec4 u_Params = vec4(0.5,0.0,0.0,0.0); //x: value, y:? , z:isSelected, w:segments
	//float u_AlphaMod;

	vec2 samp = abs(v_TexCoords);
	float borderRad = 10.0;
	float fillRad = 20.0;
	float cornerRad = 40.0;

	vec2 u_Dimensions = v_Data0.xy;

	vec2 sampScaled = samp;
	sampScaled.x *= u_Dimensions.x;
	sampScaled.y *= u_Dimensions.y;

	vec2 cornerPoint = vec2(u_Dimensions-cornerRad);
	vec2 edgePoint = vec2(u_Dimensions-borderRad);
	vec2 edgePointF = vec2(u_Dimensions-fillRad);
	vec2 cornerLengthVec = vec2(sampScaled.x - cornerPoint.x,sampScaled.y - cornerPoint.y);
	float cornerLength = length(cornerLengthVec);

	float isCorner = float(sampScaled.x > cornerPoint.x) * float(sampScaled.y > cornerPoint.y);
	float isNotCorner = 1.0-isCorner;
	float isOpaque = min(float(cornerLength/cornerRad <= 1.0) + isNotCorner,1.0);

	float isEdgeBorder = min(float(sampScaled.x > edgePoint.x) + float(sampScaled.y > edgePoint.y),1.0);
	float isCornerBorder = float(cornerLength > cornerRad-borderRad);
	float isBorder = mix(isEdgeBorder, isCornerBorder, isCorner);

	float isEdgeBorderF = min(float(sampScaled.x > edgePointF.x) + float(sampScaled.y > edgePointF.y),1.0);
	float isCornerBorderF = float(cornerLength > cornerRad-fillRad);
	float isBorderF = mix(isEdgeBorderF, isCornerBorderF, isCorner);

	float divAmount = 0.05;
	float iDivAmount = 1.0-divAmount;
	float curVal = max(v_TexCoords.x*u_Params.w, v_TexCoords.x);
	float isNotDiv = float(mod(curVal,1.0) < iDivAmount );
	isNotDiv = min(isNotDiv + float(v_TexCoords.x > iDivAmount) + float(u_Params.w <= 1.0), 1.0);


	float isFilled = float(curVal < u_Params.x)*isNotDiv;
	float isFilled2 = float(curVal < u_Params.y)/2.0;

	float toggleAmount = (1.0-isBorderF)*(isFilled-isFilled2);
	
	//vec2 coords1 = vec2(v_TexCoords.y,u_TexLookup);
	//vec2 coords2 = vec2(v_TexCoords.y,u_TexLookup + 1.0/255.0);
	//vec4 ltex = texture2D(u_Texture1, coords2);
	//vec4 dtex = texture2D(u_Texture1, coords1);
	
	vec4 ltex = vec4(1.0,0.0,0.0,1.0);
	vec4 dtex = vec4(0.5,0.0,0.0,1.0);
	vec4 res = mix(dtex,ltex,toggleAmount);

	//float isStripe = float(mod(v_TexCoords.x*40.0 + v_TexCoords.y*20.0 , 2.0) > 1.0)/16.0;

	vec3 borderCol = vec3((cornerLengthVec),0.0);

	vec4 colBG;
	colBG.xyz = mix(res.xyz, borderCol, isBorder);// + u_Params.z*abs(sin(u_SysTime/40.0))/3.0;  res.xyz + isStripe
	colBG.w = mix(0.5 + 0.5*toggleAmount,isOpaque,isBorder);// *u_AlphaMod;
	*/

	
	vec4 heightRT = texture2D( u_Texture0, v_TexCoords );
	vec4 normRT = texture2D( u_Texture1, v_TexCoords );
	vec3 norm = vec3((normRT.rg-0.5)*2.0,0.0);
	vec3 faceNorm = vec3(0.0,0.0,1.0);
	float curDis = heightRT.b;
	float maxDis = 128.0/255.0;
	float startZ = 0.0;
	float endZ = 1.0;
	float lerpPow = 2.0;
	float lerpAmount = max(maxDis-curDis,0.0)/maxDis;
	if (curDis <= 32.0/255.0) {
		lerpAmount = 1.0;
	}
	vec3 finalNorm = mix(faceNorm, norm, pow(lerpAmount,lerpPow) );


	float alphaVal = heightRT.g;

	
	if (alphaVal != 0.0) {
		alphaVal = 1.0;
	}
	

	/*
	vec3 lightVec;
	float disVal = 1.0-clamp(distance(v_MouseCoords, v_Position)*2.0/u_Zoom,0.0,1.0);
	lightVec.xy = v_MouseCoords - v_Position;
	lightVec.z = 0.1;
	lightVec = normalize(lightVec);
	float lVal = dot(finalNorm,lightVec)*disVal;
	gl_FragColor = vec4(lVal,lVal,lVal,heightRT.g);
	*/
	
	gl_FragColor = vec4((finalNorm.xy+1.0)/2.0, 20.0/255.0, alphaVal );
	
	
}