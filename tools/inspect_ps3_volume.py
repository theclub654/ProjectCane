import json
import traceback
import renderdoc as rd

OUTPUT = r"C:\Users\denar\source\repos\ProjectCane\tools\ps3_volume_inspection_v2.json"
TARGET_EVENTS = (4588, 4599, 4611)


def flatten(actions):
    result = []
    for action in actions:
        result.append(action)
        result.extend(flatten(action.children))
    return result


def scalar(value):
    if value is None or isinstance(value, (bool, int, float, str)):
        return value
    try:
        return int(value)
    except Exception:
        return str(value)


def fields(obj, names):
    out = {}
    for name in names:
        try:
            out[name] = scalar(getattr(obj, name))
        except Exception as exc:
            out[name] = "<error: {}>".format(exc)
    return out


def descriptor_resource(descriptor):
    for name in ("resourceId", "resource"):
        try:
            return scalar(getattr(descriptor, name))
        except Exception:
            pass
    return str(descriptor)


def inspect(controller):
    actions = flatten(controller.GetRootActions())
    by_event = {action.eventId: action for action in actions}
    result = {
        "api": str(controller.GetAPIProperties().pipelineType),
        "nearby_actions": [],
        "matching_actions": [],
        "events": [],
    }

    for action in actions:
        if 5650 <= action.eventId <= 5710:
            result["nearby_actions"].append(fields(action, (
                "eventId", "actionId", "customName", "numIndices", "numInstances",
                "indexOffset", "vertexOffset", "instanceOffset", "flags",
            )))
        if action.numIndices in (138, 156):
            result["matching_actions"].append(fields(action, (
                "eventId", "actionId", "customName", "numIndices", "numInstances",
                "indexOffset", "vertexOffset", "instanceOffset", "flags",
            )))

    for event_id in TARGET_EVENTS:
        action = by_event.get(event_id)
        if action is None:
            prior = [candidate for candidate in actions if candidate.eventId < event_id]
            action = prior[-1] if prior else None

        controller.SetFrameEvent(event_id, True)
        pipe = controller.GetPipelineState()
        gl = controller.GetGLPipelineState()
        event = {
            "requestedEventId": event_id,
            "action": fields(action, (
                "eventId", "actionId", "customName", "numIndices", "numInstances",
                "indexOffset", "vertexOffset", "instanceOffset", "flags",
            )) if action is not None else None,
            "pipeline": {
                "topology": scalar(pipe.GetPrimitiveTopology()),
                "viewport": fields(pipe.GetViewport(0), ("x", "y", "width", "height", "minDepth", "maxDepth")),
                "scissor": fields(pipe.GetScissor(0), ("x", "y", "width", "height", "enabled")),
                "depth_target": descriptor_resource(pipe.GetDepthTarget()),
                "outputs": [descriptor_resource(target) for target in pipe.GetOutputTargets()],
            },
            "gl": {},
        }

        for section in ("depthState", "stencilState", "rasterizer"):
            try:
                obj = getattr(gl, section)
                event["gl"][section] = {
                    "repr": str(obj),
                    "fields": fields(obj, tuple(name for name in dir(obj) if not name.startswith("_"))),
                }
            except Exception as exc:
                event["gl"][section] = {"error": str(exc)}

        try:
            event["gl"]["framebuffer"] = fields(
                gl.framebuffer,
                tuple(name for name in dir(gl.framebuffer) if not name.startswith("_")))
        except Exception as exc:
            event["gl"]["framebuffer"] = {"error": str(exc)}

        result["events"].append(event)

    with open(OUTPUT, "w", encoding="utf-8") as stream:
        json.dump(result, stream, indent=2)


try:
    pyrenderdoc.Replay().BlockInvoke(inspect)
except Exception:
    with open(OUTPUT, "w", encoding="utf-8") as stream:
        json.dump({"error": traceback.format_exc()}, stream, indent=2)
