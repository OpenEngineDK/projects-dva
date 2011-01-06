// 
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS)
//
// This program is free software; It is covered by the GNU General
// Public License version 2 or any later version.
// See the GNU General Public License for more details (see LICENSE).
//--------------------------------------------------------------------

#ifndef _ANIM_SEQ_NODE_H_
#define _ANIM_SEQ_NODE_H_ 

namespace OpenEngine {
namespace Scene {

    /**
     * An Animation sequence represents a single clip, eg. the walking
     * cycle of a character. The animation sequence has a duration
     * measured in ticks and a ticks per second constant. Each
     * animation sequence is associated with a set of animated
     * transformations each with variable length below or equal to the
     * duration of the sequence. For ex. lets say the walking cycle
     * sequence has a duration of 10 secs and consists of two
     * channels, one for moving the legs and one for the arms.
     * The legs are animated through the entire sequence but
     * the arms are only animated during the middle 5 secs. of the
     * sequence. The key frames in each animated transformation
     * define the duration of each channel.
     */
    class AnimSeqNode : public Scene::ISceneNode {
        OE_SCENE_NODE(AnimSeqNode, ISceneNode)
        
    public:
        AnimSeqNode();
        virtual ~AnimSeqNode();

    private:
        double duration; // In ticks.
        double ticksPerSecond;
        unsigned int numAnimTrans;
        std::list<AnimTransNode*> animTrans;
    };

} // NS Scene
} // NS OpenEngine

#endif
