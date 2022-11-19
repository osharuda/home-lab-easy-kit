#ifdef {DEVNAME}_DEVICE_ENABLED
    struct {DevName}CommandHandlers {
        std::shared_ptr<{DevName}> dev;
        std::shared_ptr<CommandHandler> {devname}_info_handler;
    };

    std::vector<{DevName}CommandHandlers> {devname}_handlers({DEVNAME}_DEVICE_COUNT);

    for (size_t index=0; index<{DEVNAME}_DEVICE_COUNT; index++) {
        const {DevName}Instance* descr = {DevName}::get_descriptor(index);
        uint8_t dev_id = descr->dev_id;
        {DevName}CommandHandlers& h = {devname}_handlers.at(index);


        h.dev.reset(new {DevName}(firmware, dev_id));
        h.{devname}_info_handler.reset(dynamic_cast<CommandHandler*>(new {DevName}InfoHandler(std::dynamic_pointer_cast<EKitDeviceBase>(h.dev), ui)));
        ui->add_command(cmd_index++, h.{devname}_info_handler);  
    }
#endif  