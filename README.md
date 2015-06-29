# Config handling

A typical config loading process will be something like this:

    using namespace appcon;
    auto cfg = std::make_shared<config>();
    (*cfg)
     ("config", std::string { "streamer.cfg" }, "Config file path")
     ("log", std::string { "debug" }, "log level, default INFO")
    ;
	// We allow env-based config, used in Docker for example
    cfg->from_environment("EXAMPLE_");
	// Commandline parameters override environment
    cfg->from_args(argc, argv);
	// Config file support
    cfg->from_file(
	  /* Try to load from --config / EXAMPLE_CONFIG
	   * if available...
	  cfg->key("config"),
	  /* ... but we don't mind too much if there's no file */
	  config::ignore_missing
	);
	// Throw an exception if a config value is used
	// that we don't already know about, or if it
	// is declared with a default that does not
	// match the existing default
    cfg->strict(true);

