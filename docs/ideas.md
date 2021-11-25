

### Requirements
* prefabs (unity) / actor blueprints (unreal)
* environment painting with automatic instantiation (object painting)
* cave system / multi-layer heightmaps
* drag- and drop (asset explorere <-> scene)
* reordering of assets in explorer (moving assets from folders...)
* object oriented actors ( not ecs -> create entity + add components )
* default instantiate default classes with actor attributes (serialized)
* make derived classes of default actor classes possible
* multi derived classes of graphics, physical and audio actors classes
* (( plexing of fiber/grass objects to use curves or other response elements ))

### Update Implications
* engine internal staged from user data
> User Update | direct/write (write into data) <-> indirect/patch (writes into buffer)