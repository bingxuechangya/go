package czm

/*
#include "czm-log.h"
int verbose_flag = 1;
*/
import "C"
import "unsafe"

func Cstr(str string) *C.char {
	// 如果把 C.char 换成 byte,那么需要再加一层 (*C.char)(unsafe.Pointer()) 做地址转换。
	return (*C.char)(unsafe.Pointer(&[]byte(str + "\000")[0]))
}

func Dump(label string, buf []byte, bufLen int) {
	C.czm_dump(Cstr(label), (*C.char)(unsafe.Pointer(&buf[0])), C.size_t(bufLen))
}

func DumpS(label string, buf string) {
	Dump(label, []byte(buf), len(buf))
}
