#
# os/windows/x86_64/fpu/fpu_cmp_f_64.asm
#
# This file is subject to the terms and conditions defined in
# 'LICENSE', which is part of this source code package.
#

.code
fpu_cmp_f_64 proc
  movsd xmm0, QWORD PTR [rcx]
  movsd xmm1, QWORD PTR [rdx]
  comisd xmm1, xmm0
  ret
fpu_cmp_f_64 endp
end

