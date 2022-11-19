//----------------------------------------------------------------------------------------------//
//                                    {DevName}InfoHandler                                      //
//----------------------------------------------------------------------------------------------//
DEFINE_HANDLER_DEFAULT_IMPL({DevName}InfoHandler,"{devname}::", "::info")
std::string {DevName}InfoHandler::help() const {
    auto d = dynamic_cast<{DevName}*>(device.get());
    return tools::format_string("# %s shows information for %s device. No parameters are required.\n",
                                get_command_name(), 
                                d->get_dev_name());
}

void {DevName}InfoHandler::handle(const std::vector<std::string>& args) {
    auto d = dynamic_cast<{DevName}*>(device.get());
    ui->log(tools::str_format("Not implemented"));
}
