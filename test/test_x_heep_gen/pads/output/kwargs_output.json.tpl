<%!
    import json
    from enum import Enum

    def make_json_serializable(obj):
        if isinstance(obj, (str, int, float, bool)) or obj is None:
            return obj
        if isinstance(obj,Enum):
            return obj.value 
        if isinstance(obj, dict):
            return {k: make_json_serializable(v) for k, v in obj.items()}
        if isinstance(obj, (list, tuple, set)):
            return [make_json_serializable(x) for x in obj]
        if hasattr(obj, "__dict__"):
            return {k: make_json_serializable(v) for k, v in obj.__dict__.items()}
        return str(obj)
%>

{
    "pad_list": ${json.dumps(make_json_serializable(xheep.get_padring().pad_list))},
    "external_pad_list": ${json.dumps(make_json_serializable(xheep.get_padring().external_pad_list))},
    "total_pad_list": ${json.dumps(make_json_serializable(xheep.get_padring().total_pad_list))},
    "total_pad": ${json.dumps(make_json_serializable(xheep.get_padring().total_pad))},
    "right_pad_list": ${json.dumps(make_json_serializable(xheep.get_padring().right_pad_list))},
    "left_pad_list": ${json.dumps(make_json_serializable(xheep.get_padring().left_pad_list))},
    "top_pad_list": ${json.dumps(make_json_serializable(xheep.get_padring().top_pad_list))},
    "bottom_pad_list": ${json.dumps(make_json_serializable(xheep.get_padring().bottom_pad_list))},
    "physical_attributes": ${json.dumps(make_json_serializable(xheep.get_padring().physical_attributes))},
    "bondpad_offsets": ${json.dumps(make_json_serializable(xheep.get_padring().bondpad_offsets))},
    "pad_constant_driver_assign": ${json.dumps(make_json_serializable(xheep.get_padring().pad_constant_driver_assign))},
    "pad_mux_process": ${json.dumps(make_json_serializable(xheep.get_padring().pad_mux_process))},
    "pad_muxed_list": ${json.dumps(make_json_serializable(xheep.get_padring().pad_muxed_list))},
    "total_pad_muxed": ${json.dumps(make_json_serializable(xheep.get_padring().total_pad_muxed))},
    "max_total_pad_mux_bitlengh": ${json.dumps(make_json_serializable(xheep.get_padring().max_total_pad_mux_bitlengh))},
    "pads_attributes": ${json.dumps(make_json_serializable(xheep.get_padring().pads_attributes))}
}
