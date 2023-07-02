//
//  cpu_relex.h
//  vsys
//
//  Created by 薯条 on 17/12/24.
//  Copyright © 2017年 薯条. All rights reserved.
//
#ifndef __vsys_cpu_relax__ 
#define __vsys_cpu_relax__

/**
 * cpu_relax is an architecture specific method of telling the CPU that you don't want it to
 * do much work. asm volatile keeps the compiler from optimising these instructions out.
 */
#if defined(__i386__) || defined(__x86_64__)
#define cpu_relax() asm volatile("rep; nop" ::: "memory");

#elif defined(__arm__) || defined(__mips__)
#define cpu_relax() asm volatile("":::"memory")

#elif defined(__aarch64__)
#define cpu_relax() asm volatile("yield" ::: "memory")

#else
#error "cpu_relax is not defined for this architecture"
#endif


#endif /* __vsys_cpu_relax__ */
