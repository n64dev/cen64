;
; os/windows/x86_64/fpu/fpu_cmp_olt_32.asm
;
; This file is subject to the terms and conditions defined in
; 'LICENSE', which is part of this source code package.
;

.code
fpu_cmp_olt_32 proc
  movss xmm0, DWORD PTR [rcx]
  movss xmm1, DWORD PTR [rdx]
  comiss xmm1, xmm0
  seta dl
  setnp al
  and al, dl
  ret
fpu_cmp_olt_32 endp
end

