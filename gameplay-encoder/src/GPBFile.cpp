#include "Base.h"
#include "GPBFile.h"
#include "Transform.h"

#define EPSILON 1.2e-7f;

namespace gameplay
{

static GPBFile* __instance = NULL;

/**
 * Returns true if the given value is close to one.
 */
static bool isAlmostOne(float value);

GPBFile::GPBFile(void)
    : _file(NULL), _animationsAdded(false)
{
    __instance = this;
}

GPBFile::~GPBFile(void)
{
}

GPBFile* GPBFile::getInstance()
{
    return __instance;
}

void GPBFile::saveBinary(const std::string& filepath)
{
    _file = fopen(filepath.c_str(), "w+b");

    // identifier
    char identifier[] = { '�', 'G', 'P', 'B', '�', '\r', '\n', '\x1A', '\n' };
    fwrite(identifier, 1, sizeof(identifier), _file);

    // version
    fwrite(GPB_VERSION, 1, sizeof(GPB_VERSION), _file);

    // write refs
    _refTable.writeBinary(_file);

    // meshes
    write(_geometry.size(), _file);
    for (std::list<Mesh*>::const_iterator i = _geometry.begin(); i != _geometry.end(); ++i)
    {
        (*i)->writeBinary(_file);
    }

    // Objects
    write(_objects.size(), _file);
    for (std::list<Object*>::const_iterator i = _objects.begin(); i != _objects.end(); ++i)
    {
        (*i)->writeBinary(_file);
    }

    _refTable.updateOffsets(_file);
    
    fclose(_file);
}

void GPBFile::saveText(const std::string& filepath)
{
    _file = fopen(filepath.c_str(), "w");

    fprintf(_file, "<root>\n");

    // write refs
    _refTable.writeText(_file);

    // meshes
    for (std::list<Mesh*>::const_iterator i = _geometry.begin(); i != _geometry.end(); ++i)
    {
        (*i)->writeText(_file);
    }

    // Objects
    for (std::list<Object*>::const_iterator i = _objects.begin(); i != _objects.end(); ++i)
    {
        (*i)->writeText(_file);
    }

    fprintf(_file, "</root>");

    fclose(_file);
}

void GPBFile::add(Object* obj)
{
    _objects.push_back(obj);
}

void GPBFile::addScene(Scene* scene)
{
    addToRefTable(scene);
    _objects.push_back(scene);
}

void GPBFile::addCamera(Camera* camera)
{
    addToRefTable(camera);
    _cameras.push_back(camera);
}

void GPBFile::addLight(Light* light)
{
    addToRefTable(light);
    _lights.push_back(light);
}

void GPBFile::addMesh(Mesh* mesh)
{
    addToRefTable(mesh);
    _geometry.push_back(mesh);
}

void GPBFile::addNode(Node* node)
{
    addToRefTable(node);
    _nodes.push_back(node);
}

void GPBFile::addScenelessNode(Node* node)
{
    addToRefTable(node);
    _nodes.push_back(node);
    // Nodes are normally written to file as part of a scene. 
    // Nodes that don't belong to a scene need to be written on their own (outside a scene).
    // That is why node is added to the list of objects here.
    _objects.push_back(node);
}

void GPBFile::addAnimation(Animation* animation)
{
    _animations.add(animation);

    if (!_animationsAdded)
    {
        // The animations container should only be added once and only if the file has at least one animation.
        _animationsAdded = true;
        addToRefTable(&_animations);
        add(&_animations);
    }
}

void GPBFile::addToRefTable(Object* obj)
{
    if (obj)
    {
        const std::string& id = obj->getId();
        if (id.length() > 0)
        {
            if (_refTable.get(id) == NULL)
            {
                _refTable.add(id, obj);
            }
        }
    }
}

Object* GPBFile::getFromRefTable(const std::string& id)
{
    return _refTable.get(id);
}

bool GPBFile::idExists(const std::string& id)
{
    return _refTable.get(id) != NULL;
}

Camera* GPBFile::getCamera(const char* id)
{
    if (!id)
        return NULL;
    // TODO: O(n) search is not ideal
    for (std::list<Camera*>::const_iterator i = _cameras.begin(); i != _cameras.end(); ++i)
    {
        const std::string& _id = (*i)->getId();
        if (_id.length() > 0 && strncmp(id, _id.c_str(), 255) == 0)
        {
            return *i;
        }
    }
    return NULL;
}

Light* GPBFile::getLight(const char* id)
{
    if (!id)
        return NULL;
    // TODO: O(n) search is not ideal
    for (std::list<Light*>::const_iterator i = _lights.begin(); i != _lights.end(); ++i)
    {
        const std::string& _id = (*i)->getId();
        if (_id.length() > 0 && strncmp(id, _id.c_str(), 255) == 0)
        {
            return *i;
        }
    }
    return NULL;
}

Mesh* GPBFile::getMesh(const char* id)
{
    if (!id)
        return NULL;
    // TODO: O(n) search is not ideal
    for (std::list<Mesh*>::const_iterator i = _geometry.begin(); i != _geometry.end(); ++i)
    {
        const std::string& _id = (*i)->getId();
        if (_id.length() > 0 && strncmp(id, _id.c_str(), 255) == 0)
        {
            return *i;
        }
    }
    return NULL;
}

Node* GPBFile::getNode(const char* id)
{
    if (!id)
        return NULL;
    // TODO: O(n) search is not ideal
    for (std::list<Node*>::const_iterator i = _nodes.begin(); i != _nodes.end(); ++i)
    {
        const std::string& _id = (*i)->getId();
        if (_id.length() > 0 && strncmp(id, _id.c_str(), 255) == 0)
        {
            return *i;
        }
    }
    return NULL;
}

Animations* GPBFile::getAnimations()
{
    return &_animations;
}

void GPBFile::adjust()
{
    // calculate the ambient color for each scene
    for (std::list<Object*>::iterator i = _objects.begin(); i != _objects.end(); ++i)
    {
        Object* obj = *i;
        if (obj->getTypeId() == Object::SCENE_ID)
        {
            Scene* scene = dynamic_cast<Scene*>(obj);
            scene->calcAmbientColor();
        }
    }

    for (std::list<Node*>::const_iterator i = _nodes.begin(); i != _nodes.end(); ++i)
    {
        computeBounds(*i);
    }

    // try to convert joint transform animations into rotation animations
    //optimizeTransformAnimations();

    // TODO:
    // remove ambient _lights
    // for each node
    //   if node has ambient light
    //     if node has no camera, mesh or children but 1 ambient light
    //       delete node and remove from ref table
    //     delete light and remove from ref table
    //
    // merge animations if possible
    //   Search for animations that have the same target and key times and see if they can be merged.
    //   Blender will output a simple translation animation to 3 separate animations with the same key times but targetting X, Y and Z.
    //   This can be merged into one animation. Same for scale animations.
}

void GPBFile::computeBounds(Node* node)
{
    assert(node);
    if (Model* model = node->getModel())
    {
        if (Mesh* mesh = model->getMesh())
        {
            mesh->computeBounds();
        }
        if (MeshSkin* skin = model->getSkin())
        {
            skin->computeBounds();
        }
    }
    for (Node* child = node->getFirstChild(); child != NULL; child = child->getNextSibling())
    {
        computeBounds(child);
    }
}

void GPBFile::optimizeTransformAnimations()
{
    const unsigned int animationCount = _animations.getAnimationCount();
    for (unsigned int animationIndex = 0; animationIndex < animationCount; ++animationIndex)
    {
        Animation* animation = _animations.getAnimation(animationIndex);
        assert(animation);
        const int channelCount = animation->getAnimationChannelCount();
        // loop backwards because we will be adding and removing channels
        for (int channelIndex = channelCount -1; channelIndex >= 0 ; --channelIndex)
        {
            AnimationChannel* channel = animation->getAnimationChannel(channelIndex);
            assert(channel);
            // get target node
            const Object* obj = _refTable.get(channel->getTargetId());
            if (obj && obj->getTypeId() == Object::NODE_ID)
            {
                const Node* node = static_cast<const Node*>(obj);
                if (node->isJoint() && channel->getTargetAttribute() == Transform::ANIMATE_SCALE_ROTATE_TRANSLATE)
                {
                    decomposeTransformAnimationChannel(animation, channel);

                    animation->remove(channel);
                    SAFE_DELETE(channel);
                }
            }
        }
    }
}


void GPBFile::decomposeTransformAnimationChannel(Animation* animation, const AnimationChannel* channel)
{
    const std::vector<float>& keyTimes = channel->getKeyTimes();
    const std::vector<float>& keyValues = channel->getKeyValues();
    const size_t keyTimesSize = keyTimes.size();
    const size_t keyValuesSize = keyValues.size();

    std::vector<float> scaleKeyValues;
    std::vector<float> rotateKeyValues;
    std::vector<float> translateKeyValues;
                    
    scaleKeyValues.reserve(keyTimesSize * 3);
    rotateKeyValues.reserve(keyTimesSize * 4);
    translateKeyValues.reserve(keyTimesSize * 3);

    for (size_t kv = 0; kv < keyValuesSize; kv += 10)
    {
        scaleKeyValues.push_back(keyValues[kv]);
        scaleKeyValues.push_back(keyValues[kv+1]);
        scaleKeyValues.push_back(keyValues[kv+2]);

        rotateKeyValues.push_back(keyValues[kv+3]);
        rotateKeyValues.push_back(keyValues[kv+4]);
        rotateKeyValues.push_back(keyValues[kv+5]);
        rotateKeyValues.push_back(keyValues[kv+6]);

        translateKeyValues.push_back(keyValues[kv+7]);
        translateKeyValues.push_back(keyValues[kv+8]);
        translateKeyValues.push_back(keyValues[kv+9]);
    }

    // replace transform animation channel with translate, rotate and scale animation channels

    // Don't add the scale channel if all the key values are close to 1.0
    size_t oneCount = (size_t)std::count_if(scaleKeyValues.begin(), scaleKeyValues.end(), isAlmostOne);
    if (scaleKeyValues.size() != oneCount)
    {
        AnimationChannel* scaleChannel = new AnimationChannel();
        scaleChannel->setTargetId(channel->getTargetId());
        scaleChannel->setKeyTimes(channel->getKeyTimes());
        scaleChannel->setTangentsIn(channel->getTangentsIn());
        scaleChannel->setTangentsOut(channel->getTangentsOut());
        scaleChannel->setInterpolations(channel->getInterpolationTypes());
        scaleChannel->setTargetAttribute(Transform::ANIMATE_SCALE);
        scaleChannel->setKeyValues(scaleKeyValues);
        scaleChannel->removeDuplicates();
        animation->add(scaleChannel);
    }

    AnimationChannel* rotateChannel = new AnimationChannel();
    rotateChannel->setTargetId(channel->getTargetId());
    rotateChannel->setKeyTimes(channel->getKeyTimes());
    rotateChannel->setTangentsIn(channel->getTangentsIn());
    rotateChannel->setTangentsOut(channel->getTangentsOut());
    rotateChannel->setInterpolations(channel->getInterpolationTypes());
    rotateChannel->setTargetAttribute(Transform::ANIMATE_ROTATE);
    rotateChannel->setKeyValues(rotateKeyValues);
    rotateChannel->removeDuplicates();
    animation->add(rotateChannel);

    AnimationChannel* translateChannel = new AnimationChannel();
    translateChannel->setTargetId(channel->getTargetId());
    translateChannel->setKeyTimes(channel->getKeyTimes());
    translateChannel->setTangentsIn(channel->getTangentsIn());
    translateChannel->setTangentsOut(channel->getTangentsOut());
    translateChannel->setInterpolations(channel->getInterpolationTypes());
    translateChannel->setTargetAttribute(Transform::ANIMATE_TRANSLATE);
    translateChannel->setKeyValues(translateKeyValues);
    translateChannel->removeDuplicates();
    animation->add(translateChannel);
}

static bool isAlmostOne(float value)
{
    return std::abs(value - 1.0f) < EPSILON;
}

}
