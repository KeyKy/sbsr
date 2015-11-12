// stdafx.cpp : 只包括标准包含文件的源文件
// sbsr.pch 将作为预编译头
// stdafx.obj 将包含预编译类型信息

#include "stdafx.h"

// TODO: 在 STDAFX.H 中
// 引用任何所需的附加头文件，而不是在此文件中引用

#ifdef _DEBUG
	#pragma  comment(lib,"opencv_core220d.lib")
	#pragma  comment(lib,"opencv_highgui220d.lib")
	#pragma  comment(lib,"opencv_imgproc220d.lib")
#else
	#pragma  comment(lib,"opencv_core220.lib")
	#pragma  comment(lib,"opencv_highgui220.lib")
	#pragma  comment(lib,"opencv_imgproc220.lib")
#endif
