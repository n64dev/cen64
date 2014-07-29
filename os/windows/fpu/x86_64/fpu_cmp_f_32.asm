.code
fpu_cmp_f_32 proc
  mov DWORD PTR [rsp-4], ecx
  movss xmm0, DWORD PTR [rsp-4]
  mov DWORD PTR [rsp-4], edx
  movss xmm1, DWORD PTR [rsp-4]
  comiss xmm1, xmm0
  ret
fpu_cmp_f_32 endp
end

