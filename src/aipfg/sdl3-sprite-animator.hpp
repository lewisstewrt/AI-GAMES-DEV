#ifndef _SDL3_SPRITE_ANIMATOR_HPP_
#define _SDL3_SPRITE_ANIMATOR_HPP_

#include "sdl3-sprite-sheet.hpp"

class SpriteAnimator
{
public:
  explicit SpriteAnimator(const SpriteSheet& atlas)
    : atlas_{atlas}, current_frame_{0}, elapsed_{0.0f}, tag_{nullptr},
      ping_pong_forward_{true}
  {
  }

  void play(const std::string& tag_name)
  {
    const FrameTag* tag = atlas_.find_tag(tag_name);
    if (tag)
    {
      tag_               = tag;
      current_frame_     = tag->from;
      elapsed_           = 0.0f;
      ping_pong_forward_ = true;
    }
  }

  void update(float dt_ms)
  {
    if (!tag_ || tag_->from == tag_->to)
    {
      return;
    }

    elapsed_ += dt_ms;

    const Frame& frame    = atlas_.get_frame(current_frame_);
    float        duration = static_cast<float>(frame.duration);

    while (elapsed_ >= duration)
    {
      elapsed_ -= duration;
      advance_frame();
    }
  }

  size_t current_frame() const { return current_frame_; }

  void render(SDL_Renderer* renderer, float x, float y,
              float scale = 1.0f) const
  {
    atlas_.render_frame(renderer, current_frame_, x, y, scale);
  }

private:
  void advance_frame()
  {
    if (!tag_)
    {
      return;
    }

    if (tag_->direction == "forward")
    {
      current_frame_++;
      if (current_frame_ > static_cast<size_t>(tag_->to))
      {
        current_frame_ = tag_->from;
      }
    }
    else if (tag_->direction == "reverse")
    {
      if (current_frame_ <= static_cast<size_t>(tag_->from))
      {
        current_frame_ = tag_->to;
      }
      else
      {
        current_frame_--;
      }
    }
    else if (tag_->direction == "pingpong")
    {
      if (ping_pong_forward_)
      {
        current_frame_++;
        if (current_frame_ >= static_cast<size_t>(tag_->to))
        {
          ping_pong_forward_ = false;
        }
      }
      else
      {
        if (current_frame_ <= static_cast<size_t>(tag_->from))
        {
          ping_pong_forward_ = true;
        }
        else
        {
          current_frame_--;
        }
      }
    }
  }

  const SpriteSheet& atlas_;
  size_t             current_frame_;
  float              elapsed_;
  const FrameTag*    tag_;
  bool               ping_pong_forward_;
};

#endif // _SDL3_SPRITE_ANIMATOR_HPP_
