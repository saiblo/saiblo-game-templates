var PickerRaycast = pc.createScript('pickerRaycast');

PickerRaycast.prototype.initialize = function() {
    const {touch, mouse, systems} = this.app;
    const {camera} = this.entity;

    // This prevents that a mouse click event will be executed after a touch event.
    if (touch) touch.on(pc.EVENT_TOUCHEND, ({event}) => event.preventDefault());

    mouse.on(pc.EVENT_MOUSEDOWN, ({x, y}) => {
        const from = camera.screenToWorld(x, y, camera.nearClip);
        const to = camera.screenToWorld(x, y, camera.farClip);
        const result = systems.rigidbody.raycastFirst(from, to);
        if (result) result.entity.fire('click');
    });
};