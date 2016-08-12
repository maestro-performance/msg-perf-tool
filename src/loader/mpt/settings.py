import ConfigParser

config = ConfigParser.RawConfigParser();
in_opts = {};

def read_param(section, name, default=None):
    ret = in_opts[name]
    if ret is None:
        try:
            ret = config.get(section, name)
            if ret is None and default is not None:
                ret = default
        except:
            ret = None

    return ret;


def settings_init(config_file, test_config_file):
    if config_file is not None and test_config_file is None:
        config.read(config_file)

    if config_file is None and test_config_file is not None:
        config.read(test_config_file)

    if config_file is not None and test_config_file is not None:
        config.read( [config_file, test_config_file])