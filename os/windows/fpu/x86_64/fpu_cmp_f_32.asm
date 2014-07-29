.code
fpu_cmp_f_32 proc
  movss xmm0, DWORD PTR [rdx]
  movss xmm1, DWORD PTR [rdx]
  comiss xmm1, xmm0
  ret
fpu_cmp_f_32 endp
end

