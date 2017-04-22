/*
 * A CLK test device/driver
 * Copyright (C) 2017 Synopsys
 * Author: Vlad Zakharov <vzakhar@synopsys.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/string.h>
#include <linux/err.h>
#include <linux/rtc.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/of.h>

#define pr_log(msg)	pr_info("CLK_TEST: %s\n", (msg))

/* What is it for? */
static int clktest_run_set(const char *val, const struct kernel_param *kp);
static int clktest_run_get(char *val, const struct kernel_param *kp);
static const struct kernel_param_ops run_ops = {
	.set = clktest_run_set,
	.get = clktest_run_get,
};
static char *clktest_run = "success";
module_param_cb(test_clk, &run_ops, &clktest_run, 0644);
MODULE_PARM_DESC(test_clk, "Test struct for clk");

static int clktest_run_get(char *val, const struct kernel_param *kp)
{
	return param_get_charp(val, kp);
}

static int start_clk_test(void)
{
	struct clk *clk;
	int ret;
	unsigned long freq;
	struct device_node *np;

	np = of_find_node_by_path("/timer0");
	if(IS_ERR(np)) {
		pr_log("of_find_node_by_name failed");
		return -1;
	}
	else
		pr_log("of_find_node_by_name succeed");

	clk = of_clk_get(np, 0);
	/* replace with macro? */
	if (IS_ERR(clk)) {
		pr_log("clk_get failed");
		return -1;
	}
	else
		pr_log("clk_get succeed");

	clk_prepare(clk);
	pr_log("clk_prepare succeed");

	ret = clk_enable(clk);
	if (ret) {
		pr_log("clk_enable failed");
		return -1;
	}
	else
		pr_log("clk_enable succeed");

	freq = clk_get_rate(clk);
	/*TODO: update this */
	pr_info("CLK_TEST: clk_get_rate: curr freq is %lu\n", freq);
	
	ret = clk_set_rate(clk, 0);
	if (ret) {
		pr_log("clk_set_rate failed");
		return -1;
	}
	else
		pr_log("clk_set_rate succeed");

	freq = clk_get_rate(clk);
	/*TODO: update this */
	pr_info("CLK_TEST: clk_get_rate: new freq is %lu\n", freq);

	return 0;
}

static int clktest_run_set(const char *val, const struct kernel_param *kp)
{
	start_clk_test();
	return 0;
}


static int __init test_init(void)
{
	pr_info("%s: initing test clock framework\n", __func__);
	return 0;
}


static void __exit test_exit(void)
{
	pr_info("%s: exiting test clock framework\n", __func__);
}

MODULE_AUTHOR("Vlad Zakharov <vzakhar@synopsys.com>");
MODULE_DESCRIPTION("CLK test");
MODULE_LICENSE("GPL");

module_init(test_init);
module_exit(test_exit);
