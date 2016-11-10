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
	//Entrada se transfiere a la salida
	Output[uint2(id.xy)] = Input[id.xy];

	
	//float4 A, B;
	//A = B; //Copia de dos vectores del mismo tamaño
	//float3 C;
	//A.xyz = C; //Si son de diferente tamaño se debe especificar que se quiere copiar
	//A.xyz = C.xxx; //DIFUSION(caso especial de permutacion) Permutacion y asignacion (multidifusion en varios  valores del destino)
	//A = B.(wzyx * 4) + 3;

	//A.rgba = B.xyzw;
}