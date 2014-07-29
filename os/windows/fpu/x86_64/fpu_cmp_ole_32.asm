.code
fpu_cmp_ole_32 proc
  mov DWORD PTR [rsp-4], ecx
  movss xmm0, DWORD PTR [rsp-4]
  mov DWORD PTR [rsp-4], edx
  movss xmm1, DWORD PTR [rsp-4]
  comiss xmm1, xmm0
  setae dl
  setnp al
  and al, dl
  ret
fpu_cmp_ole_32 endp
end

