var TextMeshAnimation = pc.createScript('textMeshAnimation');

TextMeshAnimation.attributes.add('offset', {
    type: 'number',
    default: 100
});
TextMeshAnimation.attributes.add('order', {
    type: 'number',
    enum: [
        { 'Left to Right': 0 },
        { 'Right to Left': 1 },
        { 'From Center': 2 },
        { 'To Center': 3 }
    ],
    default: 0
});

TextMeshAnimation.prototype.postInitialize = function() {
    var schema = this.entity.script.tween.tweens;

    var characters = this.entity.script.textMesh.characters;
    var delay = 0;
    for (var i = 0; i < characters.length; i++) {
        schema[0].delay = delay;
        schema[1].delay = delay;
        delay += this.offset;

        var character = characters[i];
        character.addComponent('script');
        character.script.create('tween', {
            attributes: {
                tweens: schema
            }
        });
    }
};
