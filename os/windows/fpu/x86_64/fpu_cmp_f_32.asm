.code
fpu_cmp_f_32 proc
  mov DWORD PTR [rsp-0x4], ecx
  movss xmm0, QWORD PTR [rsp-0x4]
  mov DWORD PTR [rsp-0x4], edx
  movss xmm1, QWORD PTR [rsp-0x4]
  comiss xmm1, xmm0
  ret
fpu_cmp_f_32 endp
end

