/*
 * de10_nano_AD193x -- SoC audio for Terasic DE10 Nano-SoC board
 * Author: T. B. Davis 
 *
 * Based on de1-soc-wm8731 by
 * Author: B. Steinsbo <bsteinsbo@gmail.com>
 *
 * Licensed under the GPL-2.
 */
 
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/clk.h>
#include <linux/platform_device.h>
#include <linux/of.h>
#include <linux/gpio.h>
#include <linux/of_gpio.h>

#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/soc.h>

#include <linux/delay.h>

#define AD193x_SYSCLK_XTAL 1
#define AD193x_SYSCLK_MCLK 2
#define MCLK_RATE_48K 12288000 /* fs*256 */
#define MCLK_RATE_44K 12288000 /* fs*384 */

static int de10AMinisoc_hw_params(struct snd_pcm_substream *substream,
  struct snd_pcm_hw_params *params)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->codec_dai;
	struct device *dev = rtd->card->dev;
	unsigned int mclk_freq;
	int ret;

	if ((params_rate(params) % 44100) == 0) {
		mclk_freq = MCLK_RATE_44K;
	} else if ((params_rate(params) % 48000) == 0) {
		mclk_freq = MCLK_RATE_48K;
	} else
		return -EINVAL;

	/* set codec mclk configuration */
	ret = snd_soc_dai_set_sysclk(codec_dai, AD193x_SYSCLK_MCLK,
		mclk_freq, SND_SOC_CLOCK_OUT);
	if (ret < 0)
		return ret;

	dev_dbg(dev, "hw_params: mclk_freq=%d\n", mclk_freq);
	return 0;
}

static void de10AMinisoc_shutdown(struct snd_pcm_substream *substream)
{
	struct snd_soc_pcm_runtime *rtd = substream->private_data;
	struct snd_soc_dai *codec_dai = rtd->codec_dai;
	struct device *dev = rtd->card->dev;
	int ret;

	dev_dbg(dev, "shutdown\n");
	ret = snd_soc_dai_set_sysclk(codec_dai, AD193x_SYSCLK_MCLK,
		0, SND_SOC_CLOCK_OUT);
	if (ret < 0) {
		dev_err(dev, "Failed to reset AD193x SYSCLK: %d\n", ret);
	}
}

static struct snd_soc_ops de10AMinisoc_ops = {
	.shutdown = de10AMinisoc_shutdown,
	.hw_params = de10AMinisoc_hw_params,
};

// Define the audio paths
static const struct snd_soc_dapm_widget de10AMinisoc_dapm_widgets[] = {
	SND_SOC_DAPM_LINE("Line In Jack",   NULL),
	SND_SOC_DAPM_LINE("Line Out Jack",  NULL),
};

// Define the connections
static const struct snd_soc_dapm_route intercon[] = {
//  { "ADC1IN",        NULL, "Line In Jack" },   // Line in goes to ADC2IN
	{ "ADC2IN",        NULL, "Line In Jack" },   // Line in goes to ADC2IN
	{ "Line Out Jack", NULL, "DAC1OUT"      },  // DAC1OUT goes to Line Out Jack
	{ "Line Out Jack", NULL, "DAC2OUT"      },  // DAC2OUT goes to Line Out Jack
	{ "Line Out Jack", NULL, "DAC3OUT"      },  // DAC3OUT goes to Line Out Jack
	{ "Line Out Jack", NULL, "DAC4OUT"      }   // DAC4OUT goes to Line Out Jack
};

static int de10AMinisoc_AD193x_init(struct snd_soc_pcm_runtime *rtd)
{
	struct snd_soc_dai *codec_dai = rtd->codec_dai;
	struct snd_soc_dai *cpu_dai = rtd->cpu_dai;
	struct device *dev = rtd->card->dev;
	unsigned int fmt;
	int ret;

	dev_dbg(dev, "init\n");

	fmt = SND_SOC_DAIFMT_I2S | SND_SOC_DAIFMT_NB_NF |
	      SND_SOC_DAIFMT_CBS_CFS;

	/* set cpu DAI configuration */
	ret = snd_soc_dai_set_fmt(cpu_dai, fmt);
	if (ret < 0)
		return ret;

	/* set codec DAI configuration */
	ret = snd_soc_dai_set_fmt(codec_dai, fmt);
	if (ret < 0)
		return ret;

	/* Don't let codec constraints interfere */
	ret = snd_soc_dai_set_sysclk(codec_dai, AD193x_SYSCLK_MCLK,
		0, SND_SOC_CLOCK_OUT);
	if (ret < 0) {
		dev_err(dev, "Failed to set AD193x SYSCLK: %d\n", ret);
		return ret;
	}

	return 0;
}

static struct snd_soc_dai_link de10AMinisoc_dai = {
	.name = "AD193x",
	.stream_name = "AD193x DAI",
	.cpu_dai_name = "ff200000.i2s",
	.codec_dai_name = "ad193x-hifi",
	.init = de10AMinisoc_AD193x_init,
	.platform_name = "de10soc",
	.codec_name = "spi0.0",
	.ops = &de10AMinisoc_ops,
};

static struct snd_soc_card snd_soc_de10AMinisoc = {
	.name = "DE10SOC-AD193x",
	.owner = THIS_MODULE,
	.dai_link = &de10AMinisoc_dai,
	.num_links = 1,

	.dapm_widgets = de10AMinisoc_dapm_widgets,
	.num_dapm_widgets = ARRAY_SIZE(de10AMinisoc_dapm_widgets),
	.dapm_routes = intercon,
	.num_dapm_routes = ARRAY_SIZE(intercon),
};

static int de10AMinisoc_audio_probe(struct platform_device *pdev)
{
	struct device_node *np = pdev->dev.of_node;
	struct device_node *codec_np, *cpu_np;
	struct snd_soc_card *card = &snd_soc_de10AMinisoc;

	int ret;

	if (!np) {
		return -ENODEV;
	}

	card->dev = &pdev->dev;

	/* Parse codec info */
	de10AMinisoc_dai.codec_name = NULL;
	codec_np = of_parse_phandle(np, "audio-codec", 0);
	if (!codec_np) {
		dev_err(&pdev->dev, "codec info missing\n");
		return -EINVAL;
	}

	de10AMinisoc_dai.codec_of_node = codec_np;
  //de10AMinisoc_dai.codec_of_node->name = "ad193x-hifi";

	/* Parse dai and platform info */
	de10AMinisoc_dai.cpu_dai_name = NULL;
	de10AMinisoc_dai.platform_name = NULL;
	cpu_np = of_parse_phandle(np, "i2s-controller", 0);
	if (!cpu_np) {
		dev_err(&pdev->dev, "dai and pcm info missing\n");
		return -EINVAL;
	}

	de10AMinisoc_dai.cpu_of_node = cpu_np;
	de10AMinisoc_dai.platform_of_node = cpu_np;

	of_node_put(codec_np);
	of_node_put(cpu_np);

	ret = snd_soc_register_card(card);
	if (ret) {
		dev_err(&pdev->dev, "snd_soc_register_card() failed\n");
	} 
	return ret;
}

static int de10AMinisoc_audio_remove(struct platform_device *pdev)
{
	struct snd_soc_card *card = platform_get_drvdata(pdev);
	snd_soc_unregister_card(card);

	return 0;
}

static const struct of_device_id de10AMinisoc_ad193x_dt_ids[] = {
	{ .compatible = "opencores,de10AMinisoc-ad193x-audio", },
	{ }
};

MODULE_DEVICE_TABLE(of, de10AMinisoc_ad193x_dt_ids);

static struct platform_driver de10AMinisoc_audio_driver = {
	.driver = {
		.name	= "de10AMinisoc-audio",
		.owner	= THIS_MODULE,
		.of_match_table = of_match_ptr(de10AMinisoc_ad193x_dt_ids),
	},
	.probe	= de10AMinisoc_audio_probe,
	.remove	= de10AMinisoc_audio_remove,
};

module_platform_driver(de10AMinisoc_audio_driver);

/* Module information */
MODULE_AUTHOR("Tyler Davis");
MODULE_DESCRIPTION("ALSA SoC DE10Nano-AudioMini-SoC_AD193x");
MODULE_LICENSE("GPL");
