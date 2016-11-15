//HLSL High Level Shader Language (Microsoft) lenguaje de sombreado de alto nivel

//Orientado a vectores, puro procesamiento matematico GPU(CO PROCESADOR matematico esclavo del CPU)

//Saber que identificar de hilo de utiliza
//Versiones enteras de 32 bits
//Minima unidad de ejecucion
//Cada grupo comparte recursos, cache y memoria, 


Texture2D<float4> Input;
RWTexture2D<float4> Output;

//Coordenada absoluta desde el hypercubo hasta el cubo
//Global   | Group es la local
[numthreads(16,16,1)]
void main(uint3 id:SV_DispatchThreadID, uint3 lid:SV_GroupThreadID)
{
	//Rotar 45°
	/*float2 rot = float2(
		id.x*cos(3.141592 / 4) - id.y*sin(3.141592 / 4),
		id.x*sin(3.141592 / 4) + id.y*cos(3.141592 / 4));
	Output[id.xy] = Input[int2(rot.xy)];*/
	//fin de rotar

	//Detectar bordes
	Output[id.xy] = Input[id.xy + int2(1, 0)] - Input[id.xy] + 0.5;
	//Fin de deteccion

	
	//float4 A, B;
	//A = B; //Copia de dos vectores del mismo tamaño
	//float3 C;
	//A.xyz = C; //Si son de diferente tamaño se debe especificar que se quiere copiar
	//A.xyz = C.xxx; //DIFUSION(caso especial de permutacion) Permutacion y asignacion (multidifusion en varios  valores del destino)
	//A = B.(wzyx * 4) + 3;

	//A.rgba = B.xyzw;
}