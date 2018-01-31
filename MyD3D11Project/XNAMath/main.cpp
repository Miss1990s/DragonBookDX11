#include <windows.h>
#include<xnamath.h>
#include<iostream>
using namespace std;

ostream & operator<<(ostream& os, FXMVECTOR v) {
	XMFLOAT3 dest;
	XMStoreFloat3(&dest, v);
	os << "(" << dest.x << "," << dest.y << "," << dest.z << ")";
	return os;
}

int main() {
	cout.setf(ios_base::boolalpha);

	if (!XMVerifyCPUSupport()) {
		cout << "xna math not supported " << endl;
	}
	XMVECTOR p = XMVectorZero();
	XMVECTOR q = XMVectorSplatOne();
	XMVECTOR u = XMVectorSet(1.0f, 2.0f, 3.0f, 0.0f);
	XMVECTOR v = XMVectorReplicate(-2.3f);
	XMVECTOR w = XMVectorSplatZ(u);

	cout << "p = " << p << endl;
	cout << "q = " << q << endl;
	cout << "u = " << u << endl;
	cout << "v = " << v << endl;
	cout << "w = " << w << endl;
	int i;
	cin >> i;
	return 0;
}