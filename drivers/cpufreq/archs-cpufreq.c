/*
 * Synopsys HS CPU frequency driver
 *
 * Copyright (C) 2017 Synopsys
 *
 * This file is licensed under the terms of the GNU General Public
 * License version 2. This program is licensed "as is" without any
 * warranty of any kind, whether express or implied.
 */

#include <linux/clk.h>
#include <linux/cpu.h>
#include <linux/cpufreq.h>
#include <linux/init.h>
#include <linux/kernel.h> 
#include <linux/module.h>
#include <linux/sfi.h>
#include <linux/slab.h>
#include <linux/smp.h>

#define HS_CPUFREQ_DEFAULT_RATE 33333333
#define HS_CPUFREQ_MAX_DEVIATION 1000 /* 1KHz */ /* TODO What deviation use? */

/* TODO write a function to work with deviations. Exact values is fuck! */

/* TODO Get tables from clock driver? */
/* Setting particular tables seems not to be a good idea! */
static struct cpufreq_frequency_table hs_freq_table[] = {
	{ 0, 0, 33333 },
	{ 0, 0, 50000 },
	{ 0, 0, 75000 }, 
	{ 0, 0, 80000 },  /* For NSIM */
	{ 0, 0, 90000 }, 
	{ 0, 0, 100000 },
	{ 0, 0, CPUFREQ_TABLE_END }
};

static unsigned int hs_cpufreq_get(unsigned int cpu) {
	unsigned long rate;
	struct cpufreq_policy policy;

	if(cpufreq_get_policy(&policy, cpu)) {
		pr_err("%s: failed to get policy for cpu%d device\n", __func__, cpu);
		return -EINVAL;	
	}

	rate = clk_get_rate(policy.clk) / 1000;	

	return rate;
}

static int hs_cpufreq_target_index(struct cpufreq_policy *policy, unsigned int index) {
	unsigned long rounded_rate;
	unsigned long frequency = policy->freq_table[index].frequency;

	rounded_rate = clk_round_rate(policy->clk, frequency * 1000);
	if (rounded_rate < 0) {
		pr_err("%s: failed to update rate for cpu%d\n", __func__, policy->cpu);
		return -EINVAL;
	}

	/* TODO What about this deviation? */
	if ((rounded_rate < frequency * 1000 - HS_CPUFREQ_MAX_DEVIATION) ||
			(rounded_rate > frequency * 1000 + HS_CPUFREQ_MAX_DEVIATION)) {
		pr_err("%s: failed to find accurate rate for cpu%d rate is %lu and rounded rate is %lu\n", __func__, policy->cpu, frequency * 1000, rounded_rate);
		return -EINVAL;
	}

	if (clk_set_rate(policy->clk, rounded_rate)) {
		pr_err("%s: failed to set rate for cpu%d\n", __func__, policy->cpu);
		return -EINVAL;
	}	

	return 0;
}

static int hs_cpufreq_verify(struct cpufreq_policy *policy) {
	/* TODO is that's all?*/
	return cpufreq_frequency_table_verify(policy, hs_freq_table);
}

static int hs_cpufreq_cpu_init(struct cpufreq_policy *policy) {
	int ret = 0;
	unsigned long rate;
	struct device *cpu_dev;
	struct clk *cpu_clk;

	cpu_dev = get_cpu_device(policy->cpu);
	if (!cpu_dev) {
		pr_err("%s: failed to get cpu%d device\n", __func__, policy->cpu);
		return -ENODEV;
	}
	cpu_clk = clk_get(cpu_dev, NULL);
	if (IS_ERR(cpu_clk)) {
		pr_err("%s: Cannot get cpu clock for cpu%d device\n", __func__, policy->cpu); 
		return -ENODEV; /* TODO change error number? */
	}
	ret = clk_enable(cpu_clk);
	if(ret) {
		pr_err("%s: failed enable cpu%d clk\n", __func__, policy->cpu);
		return -EINVAL; /* TODO change error number? */
	}
	pr_info("Initing cpufreq driver!\n");

	rate = clk_get_rate(cpu_clk);
	policy->cur = rate / 1000;
	policy->clk = cpu_clk;

	return cpufreq_generic_init(policy, hs_freq_table, CPUFREQ_ETERNAL);
}

static struct cpufreq_driver hs_cpufreq_driver = {
	.name = "hs-cpufreq",
	.init = hs_cpufreq_cpu_init,
	.verify = hs_cpufreq_verify,
	.target_index = hs_cpufreq_target_index,
	.get = hs_cpufreq_get,
};

static int __init hs_cpufreq_init(void) {
	int ret;
	ret = cpufreq_register_driver(&hs_cpufreq_driver);
	if (ret) {
		/* TODO process error, free resources */
	}
	return ret;
}
subsys_initcall(hs_cpufreq_init);

MODULE_AUTHOR("Vlad Zakharov <vzakhar@synopsys.com>");
MODULE_DESCRIPTION("Synopsys HS CPU frequency driver");
MODULE_LICENSE("GPL v2");
