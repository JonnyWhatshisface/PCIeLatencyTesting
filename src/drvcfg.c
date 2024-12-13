#include "driver.h"
#include "drvcfg.h"

ssize_t pcilat_config_read(struct device *dev, struct device_attribute *attr, char *buf) {
    /* Show configuration options */
    struct pcilat_priv *priv = dev_get_drvdata(dev);
    char *out = "bar %d\nloops %d\nndelay %d\n";

    return scnprintf(buf, PAGE_SIZE, out, priv->test.bar, priv->test.loops, priv->test.ndelay);
}

ssize_t pcilat_config_write(struct device *dev, struct device_attribute *attr, const char *buf, size_t count) {
    /* 
        Set configuration options for the benchmark tests.
        Invalid settings return -EINVAL.
    */
    struct pcilat_priv *priv = dev_get_drvdata(dev);
    int fields, rc, tmp;
    char arg1[200], arg2[200];
    char *check = buf; /* Yes, we discard the quantifier here. It's fine. No, really... */
    check[strlen(check)-1] = '\0';
    fields = sscanf(check, "%s %s", arg1, arg2);

    if (strcmp(arg1, "bar") == 0) {
            rc = kstrtoint(arg2, 0, &tmp);
            if (tmp >= 0 && tmp < 6 && priv->bar[tmp].addr != NULL) {
                priv->test.bar = tmp;
                dev_info(&priv->pdev->dev, "Config Set: BAR %d - Done\n", tmp);
            } else {
                dev_info(&priv->pdev->dev, "Config Set: BAR %d - Failed: not a valid bar\n", tmp);                
                return -EINVAL;
            }
    } else if (strcmp(arg1, "loops") == 0) {
            rc = kstrtoint(arg2, 0, &tmp);
            if (tmp > 0 && tmp <= 500000) {
                priv->test.loops = tmp;
                dev_info(&priv->pdev->dev, "Config Set: LOOPS %d - Done\n", tmp);
            } else {
                dev_info(&priv->pdev->dev, "Config Set: LOOPS %d - Failed: not a valid value [1-500000]\n", tmp);
                return -EINVAL;
            }
    } else if (strcmp(arg1, "ndelay") == 0) {
            rc = kstrtoint(arg2, 0, &tmp);
            if (tmp > 0 && tmp <= 50000) {
                priv->test.ndelay = tmp;
                dev_info(&priv->pdev->dev, "Config Set: NDELAY %d - Done\n", tmp);
            } else {
                dev_info(&priv->pdev->dev, "Config Set: LOOPS %d - Failed: not a valid value [1-50000]\n", tmp);
                return -EINVAL;
            }
    }

    return count;
}
