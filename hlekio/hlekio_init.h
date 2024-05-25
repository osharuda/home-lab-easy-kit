#pragma once
#include <linux/module.h>

extern void __iomem* GPIO_BASE_VA;
extern void __iomem* FUNC_SEL_VA[];
extern void __iomem* SET_VA[];
extern void __iomem* CLEAR_VA[];
extern void __iomem* LEVEL_VA[];
extern void __iomem* GPPUDCLK_VA[];
extern void __iomem* GPPUD_VA;
