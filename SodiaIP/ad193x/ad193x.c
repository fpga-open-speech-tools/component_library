/*
 * AD1939 Audio Codec driver
 *
 * Licensed under the GPL-2 or later.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/device.h>
#include <linux/regmap.h>
#include <linux/slab.h>
#include <sound/core.h>
#include <sound/pcm.h>
#include <sound/pcm_params.h>
#include <sound/initval.h>
#include <sound/soc.h>
#include <sound/tlv.h>
#include <linux/spi/spi.h>
#include <sound/soc.h>

#include "ad193x.h"

/* codec private data */
struct ad193x_priv {
	struct regmap *regmap;
	enum ad193x_type type;
	int sysclk;
	struct mutex lock;
};

static const struct reg_default ad193x_reg_defaults[] = {
	{ 0x00, 0x80 },
	{ 0x01, 0x00 },
	{ 0x02, 0x00 },
	{ 0x0E, 0x00 },
	{ 0x10, 0xC8 },
};


/*
 * AD193X volume/mute/de-emphasis etc. controls
 */
static const char * const ad193x_deemp[] = {"None", "48kHz", "44.1kHz", "32kHz"};

static SOC_ENUM_SINGLE_DECL(ad193x_deemp_enum, AD193X_DAC_CTRL2, 1,
			    ad193x_deemp);

static const DECLARE_TLV_DB_MINMAX(adau193x_tlv, -9563, 0);

static const unsigned int ad193x_sb[] = {32};

static struct snd_pcm_hw_constraint_list constr = {
	.list = ad193x_sb,
	.count = ARRAY_SIZE(ad193x_sb),
};

static const struct snd_kcontrol_new ad193x_snd_controls[] = {
	/* DAC volume control */
	SOC_DOUBLE_R_TLV("DAC1 Volume", AD193X_DAC_L1_VOL,
			AD193X_DAC_R1_VOL, 0, 0xFF, 1, adau193x_tlv),
	SOC_DOUBLE_R_TLV("DAC2 Volume", AD193X_DAC_L2_VOL,
			AD193X_DAC_R2_VOL, 0, 0xFF, 1, adau193x_tlv),
	SOC_DOUBLE_R_TLV("DAC3 Volume", AD193X_DAC_L3_VOL,
			AD193X_DAC_R3_VOL, 0, 0xFF, 1, adau193x_tlv),
	SOC_DOUBLE_R_TLV("DAC4 Volume", AD193X_DAC_L4_VOL,
			AD193X_DAC_R4_VOL, 0, 0xFF, 1, adau193x_tlv),

	/* DAC switch control */
	SOC_DOUBLE("DAC1 Switch", AD193X_DAC_CHNL_MUTE, AD193X_DACL1_MUTE,
		AD193X_DACR1_MUTE, 1, 1),
	SOC_DOUBLE("DAC2 Switch", AD193X_DAC_CHNL_MUTE, AD193X_DACL2_MUTE,
		AD193X_DACR2_MUTE, 1, 1),
	SOC_DOUBLE("DAC3 Switch", AD193X_DAC_CHNL_MUTE, AD193X_DACL3_MUTE,
		AD193X_DACR3_MUTE, 1, 1),
	SOC_DOUBLE("DAC4 Switch", AD193X_DAC_CHNL_MUTE, AD193X_DACL4_MUTE,
		AD193X_DACR4_MUTE, 1, 1),

	/* DAC de-emphasis */
	SOC_ENUM("Playback Deemphasis", ad193x_deemp_enum),
};

static const struct snd_kcontrol_new ad193x_adc_snd_controls[] = {
	/* ADC switch control */
//	SOC_DOUBLE("ADC1 Switch", AD193X_ADC_CTRL0, AD193X_ADCL1_MUTE,
//		AD193X_ADCR1_MUTE, 1, 1),
	SOC_DOUBLE("ADC2 Switch", AD193X_ADC_CTRL0, AD193X_ADCL2_MUTE,
		AD193X_ADCR2_MUTE, 1, 1),

	/* ADC high-pass filter */
	SOC_SINGLE("ADC High Pass Filter Switch", AD193X_ADC_CTRL0,
			AD193X_ADC_HIGHPASS_FILTER, 1, 0),
};

static const struct snd_soc_dapm_widget ad193x_dapm_widgets[] = {
	SND_SOC_DAPM_DAC("DAC", "Playback", SND_SOC_NOPM, 0, 0),
	SND_SOC_DAPM_PGA("DAC Output", AD193X_DAC_CTRL0, 0, 1, NULL, 0),
	SND_SOC_DAPM_SUPPLY("PLL_PWR", AD193X_PLL_CLK_CTRL0, 0, 1, NULL, 0),
	SND_SOC_DAPM_SUPPLY("SYSCLK", AD193X_PLL_CLK_CTRL0, 7, 0, NULL, 0),
	SND_SOC_DAPM_VMID("VMID"),
	SND_SOC_DAPM_OUTPUT("DAC1OUT"),
	SND_SOC_DAPM_OUTPUT("DAC2OUT"),
	SND_SOC_DAPM_OUTPUT("DAC3OUT"),
	SND_SOC_DAPM_OUTPUT("DAC4OUT"),
};

static const struct snd_soc_dapm_widget ad193x_adc_widgets[] = {
	SND_SOC_DAPM_ADC("ADC", "Capture", SND_SOC_NOPM, 0, 0),
	SND_SOC_DAPM_SUPPLY("ADC_PWR", AD193X_ADC_CTRL0, 0, 1, NULL, 0),
//	SND_SOC_DAPM_INPUT("ADC1IN"),
	SND_SOC_DAPM_INPUT("ADC2IN"),
};

static int ad193x_check_pll(struct snd_soc_dapm_widget *source,
			    struct snd_soc_dapm_widget *sink)
{
	struct snd_soc_component *component = snd_soc_dapm_to_component(source->dapm);
	struct ad193x_priv *ad193x = snd_soc_component_get_drvdata(component);

	return !!ad193x->sysclk;
}

static const struct snd_soc_dapm_route audio_paths[] = {
	{ "DAC", NULL, "SYSCLK" },
	{ "DAC Output", NULL, "DAC" },
	{ "DAC Output", NULL, "VMID" },
	{ "DAC1OUT", NULL, "DAC Output" },
	{ "DAC2OUT", NULL, "DAC Output" },
	{ "DAC3OUT", NULL, "DAC Output" },
	{ "DAC4OUT", NULL, "DAC Output" },
	{ "SYSCLK", NULL, "PLL_PWR", &ad193x_check_pll },
};

static const struct snd_soc_dapm_route ad193x_adc_audio_paths[] = {
	{ "ADC", NULL, "SYSCLK" },
	{ "ADC", NULL, "ADC_PWR" },
//	{ "ADC", NULL, "ADC1IN" },
	{ "ADC", NULL, "ADC2IN" },
};

static inline bool ad193x_has_adc(const struct ad193x_priv *ad193x)
{
  // Always return true since this codec is now specifically for the
  // FE audio projects.
	return true;
}

/*
 * DAI ops entries
 */

static int ad193x_mute(struct snd_soc_dai *dai, int mute)
{
/*
  Never mute the ad193x.
*/
	return 0;
}

static int ad193x_set_tdm_slot(struct snd_soc_dai *dai, unsigned int tx_mask,
			       unsigned int rx_mask, int slots, int width)
{
/*
  We only have 2 channels available for the Audio Mini, so we do not provide the option to change that here.
*/
	return 0;
}

static int ad193x_set_dai_fmt(struct snd_soc_dai *codec_dai,
		unsigned int fmt)
{
/* 
  The DAI format is already set elsewhere in the codec.  Since we have very strict requirements, we do not change it here.
*/
	return 0;
}

static const unsigned int ad193x_rates[] = {
	48000,
};

static const struct snd_pcm_hw_constraint_list ad193x_rate_constraint = {
	.list = ad193x_rates,
	.count = ARRAY_SIZE(ad193x_rates),
};

static int ad193x_set_dai_sysclk(struct snd_soc_dai *codec_dai,
		int clk_id, unsigned int freq, int dir)
{
	struct snd_soc_component *component = codec_dai->component;
	struct ad193x_priv *ad193x = snd_soc_component_get_drvdata(component);
	switch (freq) {
  case 0:
	case 12288000:
	case 18432000:
	case 24576000:
	case 36864000:
		ad193x->sysclk = freq;
		return 0;
	}
	return -EINVAL;
}

static int ad193x_hw_params(struct snd_pcm_substream *substream,
		struct snd_pcm_hw_params *params,
		struct snd_soc_dai *dai)
{
	struct snd_soc_component *component = dai->component;
	struct ad193x_priv *ad193x = snd_soc_component_get_drvdata(component);
/*
  For our project, we have a very specific design, so only we only use the
  12.288 MHz sample rate and 32 bit little endian words.
*/  
  // Use our default setup for the registers
  // Start the PLL
	regmap_write(ad193x->regmap, 0x00, 0x80);
  regmap_write(ad193x->regmap, 0x01, 0x00);

  regmap_write(ad193x->regmap, 0x10, 0xC8);

  // Set the sampling frequency of the DAC and ADC
  regmap_write(ad193x->regmap, 0x02, 0x00);
  regmap_write(ad193x->regmap, 0x0E, 0x00);



	return 0;
}

static const struct snd_soc_dai_ops ad193x_dai_ops = {
	.hw_params = ad193x_hw_params,
	.digital_mute = ad193x_mute,
	.set_tdm_slot = ad193x_set_tdm_slot,
	.set_sysclk	= ad193x_set_dai_sysclk,
	.set_fmt = ad193x_set_dai_fmt,
};

/* codec DAI instance */
static struct snd_soc_dai_driver ad193x_dai = {
	.name = "ad193x-hifi",
	.playback = {
		.stream_name = "Playback",
		.channels_min = 2,
		.channels_max = 8,
		.rates = SNDRV_PCM_RATE_48000,
		.formats = SNDRV_PCM_FMTBIT_S32_LE | SNDRV_PCM_FMTBIT_S16_LE |
			SNDRV_PCM_FMTBIT_S20_3LE | SNDRV_PCM_FMTBIT_S24_LE,
	},
	.capture = {
		.stream_name = "Capture",
		.channels_min = 2,
		.channels_max = 4,
		.rates = SNDRV_PCM_RATE_48000,
		.formats = SNDRV_PCM_FMTBIT_S32_LE | SNDRV_PCM_FMTBIT_S16_LE |
			SNDRV_PCM_FMTBIT_S20_3LE | SNDRV_PCM_FMTBIT_S24_LE,
	},
	.ops = &ad193x_dai_ops,
};

static int ad193x_component_probe(struct snd_soc_component *component)
{
	struct ad193x_priv *ad193x = snd_soc_component_get_drvdata(component);
	struct snd_soc_dapm_context *dapm = snd_soc_component_get_dapm(component);
	int num, ret;

	/* add adc widgets */
	num = ARRAY_SIZE(ad193x_adc_widgets);
	ret = snd_soc_dapm_new_controls(dapm,
					ad193x_adc_widgets,
					num);
	if (ret)
		return ret;

	/* add adc routes */
	num = ARRAY_SIZE(ad193x_adc_audio_paths);
	ret = snd_soc_dapm_add_routes(dapm,
				      ad193x_adc_audio_paths,
				      num);
	if (ret)
		return ret;

	return 0;
}

static const struct snd_soc_component_driver soc_component_dev_ad193x = {
	.probe			= ad193x_component_probe,
	.controls		= ad193x_snd_controls,
	.num_controls		= ARRAY_SIZE(ad193x_snd_controls),
	.dapm_widgets		= ad193x_dapm_widgets,
	.num_dapm_widgets	= ARRAY_SIZE(ad193x_dapm_widgets),
	.dapm_routes		= audio_paths,
	.num_dapm_routes	= ARRAY_SIZE(audio_paths),
	.idle_bias_on		= 1,
	.use_pmdown_time	= 1,
	.endianness		= 1,
	.non_legacy_dai_naming	= 1,
};

const struct regmap_config ad193x_regmap_config = {
	.val_bits = 8,
	.reg_bits = 16,
	.read_flag_mask = 0x09,
	.write_flag_mask = 0x08,
  .reg_defaults =  ad193x_reg_defaults,
  .num_reg_defaults = ARRAY_SIZE(ad193x_reg_defaults),
	.max_register = AD193X_NUM_REGS - 1,
};
EXPORT_SYMBOL_GPL(ad193x_regmap_config);

static int ad193x_hw_init(struct device *dev, struct ad193x_priv *ad193x)
{
	int ret = 0;
  
  // Write some defaults to the PLL
	regmap_write(ad193x->regmap, 0x00, 0x80);
  regmap_write(ad193x->regmap, 0x01, 0x00);

	return ret;
}
/** Id matching structure for use in driver/device matching */
static struct of_device_id ad1939_ids[] =
{
    {
        .compatible = "dev,ad193x"
    },
    { }
};
MODULE_DEVICE_TABLE(of,ad1939_ids);

static int ad193x_spi_probe(struct spi_device *spi)
{
	struct ad193x_priv *ad193x;
	int ret = 0;
  int ret_val = 0;

	ad193x = devm_kzalloc(&spi->dev, sizeof(*ad193x), GFP_KERNEL);
	if (ad193x == NULL)
		return -ENOMEM;

	mutex_init(&ad193x->lock);

	spi_set_drvdata(spi, ad193x);

	ad193x->regmap = devm_regmap_init_spi(spi, &ad193x_regmap_config);
	if (IS_ERR(ad193x->regmap)) {
		ret = PTR_ERR(ad193x->regmap);
		dev_err(&spi->dev, "Failed to allocate register map: %d\n",
			ret);
		return ret;
	}

  ret = ad193x_hw_init(&spi->dev,ad193x);
  if (ret != 0)
    return ret;

	if (IS_ERR(ad193x->regmap))
		return PTR_ERR(ad193x->regmap);

	spi_set_drvdata(spi, ad193x);

	ret = devm_snd_soc_register_component(&spi->dev, &soc_component_dev_ad193x,
		&ad193x_dai, 1);
  
  if (ret != 0)
  {
    dev_err(&spi->dev, "Failed to register CODEC: %d\n", ret);
  }

	return 0;

}

static struct spi_driver ad193x_spi_driver = {
	.driver = {
		.name	= "spi0",
    .of_match_table = ad1939_ids,
	},
	.probe		= ad193x_spi_probe,
	//.id_table	= ad193x_spi_id,
};
module_spi_driver(ad193x_spi_driver);

MODULE_DESCRIPTION("ASoC ad193x driver based on the code written by Barry Song <21cnbao@gmail.com>");
MODULE_AUTHOR("Tyler Davis <davis@flatearthinc.com>");
MODULE_LICENSE("GPL");

