.code
fpu_cmp_eq_32 proc
  movss xmm0, DWORD PTR [rcx]
  movss xmm1, DWORD PTR [rdx]
  comiss xmm1, xmm0
  sete dl
  setnp al
  and al, dl
  ret
fpu_cmp_eq_32 endp
end

