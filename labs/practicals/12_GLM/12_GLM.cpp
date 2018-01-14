#define GLM_ENABLE_EXPERIMENTAL
#include <glm\glm.hpp>
#include <glm\gtc\constants.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\quaternion.hpp>
#include <glm\gtx\euler_angles.hpp>
#include <glm\gtx\projection.hpp>
#include <iostream>

using namespace std;
using namespace glm;

int main() 
{
	//Exercise 1 (Define Vectors)
	vec2 a(10.0f, 0.0f);
	vec2 b(0.0f, 1.0f);
	vec3 c(0.0f, 0.0f, 10.0f);
	vec3 d(0.0f, 0.0f, -10.0f);
	vec4 e(0.0f, -20.0f, 0.0f, 0.0f);
	vec4 f(1.0f, 0.0f, 0.0f, 0.0f);
	//Exercise 2 (Define Vectors)
	vec3 a2(a, 0.0f);
	vec4 b2(b, 0.0f, 0.0f);
	vec2 c2(c);
	vec4 d2(d, 0.0f);
	vec2 e2(e);
	vec3 f2(f);
	//Excercise 3 (Addition and Substraction)
	vec2 z = a + b;
	vec2 x = a - b;
	vec3 k = c + d;
	vec3 o = c - d;
	vec4 i = e + f;
	vec4 w = e - f;
	//Exercise 4 (Multiplication and Division)
	vec2 zx = z * x;
	vec2 xz = z / x;
	vec3 ko = k * o;
	vec3 ok = k / o;
	vec4 iw = i * w;
	vec4 wi = i / w;
	//Exercise 5 (Magnitude)
	float l1 = length(zx);
	float l2 = length(ko);
	float l3 = length(wi);
	//Exercise 6 (Normalization)
	vec2 nzx = normalize(zx);
	vec3 nko = normalize(ko);
	vec4 nwi = normalize(wi);
	//Exercise 7 (Dot Product)
	float d1 = dot(zx, xz);
	float d2 = dot(ko, ok);
	float d3 = dot(iw, wi);
	//Exercise 8 (Vector Projection)
	vec2 pzx = proj(zx, xz);
	vec3 pko = proj(ko, ok);
	vec4 pwi = proj(wi, iw);
	//Exercise 9 (Cross Product)
	vec3 ccd = cross(c, d);
	vec3 cko = cross(ko, ok);
	//Exercise 10 (Defining Matrices)
	mat4 m1(1.0f);
	mat4 m2(5.0f);
	mat2x4 m3;
	mat3x4 m4 = mat3x4(
		1.0f, 2.0f, 3.0f,
		4.0f, 5.0f, 6.0f,
		7.0f, 8.0f, 9.0f,
		10.f, 11.0f, 12.0f
	);
	mat4x2 m5 = mat4x2(
		1.0f, 2.0f, 6.0f, 9.0f,
		3.0f, 4.0f, 7.0f, 3.0f
	);
	m1[0] = vec4(1.0f, -20.0f, 9.0f, 0.0f); //Sets first column to those values
	m1[3][3] = 8.0f;						//Sets the element at the 4 column and 4 row to 8.0
	m3[0] = vec4(6.0f);						//Sets the first column to 6.0
	m3[1] = vec4(2.0f);						//Sets the second column to 2.0
	mat3 m6(mat4(1.0f));
	//Exercise 11 (Addition and Substraction Of Matrixes)
	mat4 ma = m1 + m2;
	mat4 ms = m1 - m2;
	//Exercise 12 (Scaling Matrices)
	mat4 sm1 = m1 * 5.0f;
	mat4 sm2 = m2 * 85.0f;
	mat4 sm3 = m2 / 6.0f;
	mat4 sm4 = m1 / 5.0f;
	//Exercise 13 (Multiplying Matrices)
	mat4 mm1 = m1 * m2;
	mat4 mm2 = m1 * m3;
	mat4 mm3 = m2 * m3;
	mat4 mm4 = m5 * m3;
	//Exercise 14 (Matrix-Vector Multiplication)
	vec4 mv1 = e * m1;
	vec4 mv2 = f * m2;
	vec4 mv3 = vec4(c, 1.0f) * m2;
	vec4 mv4 = vec4(a, 0.0f, 1.0f)*m1;
	//Exercise 15 (Translation Matrix)
	vec3 vec(10.0f, 6.0f, -8.0f);
	mat4 mt1 = translate(mat4(1.0f), vec3(1.0f, 4.0f, 12.0f));
	vec3 result = vec3(mt1 * vec4(vec, 1.0f));
	//Exercise 16 (Rotation Matrix)
	mat4 Rx = rotate(mat4(1.0f), 3.14f, vec3(1.0f, 0.0f, 0.0f));
	mat4 Ry = rotate(mat4(1.0f), (3.14f / 2.0f), vec3(0.0f, 1.0f, 0.0f));
	mat4 Rz = rotate(mat4(1.0f), 3.14f, vec3(0.0f, 0.0f, 1.0f));
	vec3 rm1(0.0f, 1.0f, 0.0f);
	vec3 rm2(1.0f, 0.0f, 0.0f);
	vec3 r1 = vec3(Ry*vec4(rm1, 1.0f));
	vec3 r2 = vec3(Rz*vec4(rm2, 1.0f));
	//Exercise 17 (Scale Matrix)
	mat4 S1 = scale(mat4(1.0f), vec3(10.0f, 5.0f, 10.0f));
	vec3 S2(12.0f, 1.0f, 6.0f);
	vec3 s1 = vec3(S1*vec4(S2, 1.0f));
	//Exercise 18 (Combining Matrices)
	mat4 comb = mt1 * (Rx*S1);
	vec3 ex18(2.0f, 1.0f, 3.0f);
	vec3 res18 = vec3(comb*vec4(ex18, 1.0f));
	//Exercise 19 (Define Quaternion)
	quat q;
	//Exercise 20 (Quaternion Rotation)
	quat q1 = rotate(quat(), 3.14f, vec3(1.0f, 0.0f, 0.0f));
	quat q2 = rotate(quat(), 3.14f, vec3(0.0f, 1.0f, 0.0f));
	quat q3 = rotate(quat(), 3.14f, vec3(0.0f, 0.0f, 1.0f));
	//Exercise 21 (Quaternion Multiplication)
	quat R = q1 * q2*q3;
	//Exercise 22 (Conversion To Matrix)
	mat4 cm = mat4_cast(R);
	vec3 ex22(1.0f, 5.0f, 7.0f);
	vec3 re22 = vec3(cm*vec4(ex22, 1.0f));
}