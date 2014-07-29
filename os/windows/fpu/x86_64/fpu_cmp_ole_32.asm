.code
fpu_cmp_ole_32 proc
  movss xmm0, DWORD PTR [rcx]
  movss xmm1, DWORD PTR [rdx]
  comiss xmm1, xmm0
  setae dl
  setnp al
  and al, dl
  ret
fpu_cmp_ole_32 endp
end

