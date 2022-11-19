def patch(text: str, remove=False) -> str:
    macro_name = "INFO_DEV_TYPE_{DEVNAME}"
    return cxx_handle_macro_enum(text, macro_name, 'uint8_t', not remove, newline=newline)