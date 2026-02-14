#ifndef _SDL3_SPRITE_SHEET_HPP_
#define _SDL3_SPRITE_SHEET_HPP_

#include <nlohmann/json.hpp>
#include <filesystem>
#include <fstream>
#include <vector>
#include <string>

struct Frame
{
  std::string name;
  SDL_FRect   rect;     // Source rectangle in the atlas
  int         duration; // Duration in milliseconds
};

struct FrameTag
{
  std::string name;
  int         from;
  int         to;
  std::string direction; // "forward", "reverse", "pingpong"
};

class SpriteSheet
{
public:
  SpriteSheet(SDL_Renderer* renderer, const std::filesystem::path& json_path)
    : texture_{nullptr, SDL_DestroyTexture}
  {
    load_json(json_path);
    load_texture(renderer, json_path.parent_path() / image_filename_);
  }

  const Frame& get_frame(size_t index) const
  {
    if (index >= frames_.size())
    {
      throw std::out_of_range("Frame index out of range");
    }
    return frames_[index];
  }

  size_t frame_count() const { return frames_.size(); }

  const FrameTag* find_tag(const std::string& name) const
  {
    for (const auto& tag : frame_tags_)
    {
      if (tag.name == name)
      {
        return &tag;
      }
    }
    return nullptr;
  }

  SDL_Texture* texture() const { return texture_.get(); }

  void render_frame(SDL_Renderer* renderer, size_t frame_index,
                    const SDL_FRect& dest) const
  {
    if (frame_index >= frames_.size())
    {
      return;
    }
    const Frame&    frame = frames_[frame_index];
    const SDL_FRect src   = frame.rect;
    SDL_RenderTexture(renderer, texture_.get(), &src, &dest);
  }

  void render_frame(SDL_Renderer* renderer, size_t frame_index, float x,
                    float y, float scale = 1.0f) const
  {
    if (frame_index >= frames_.size())
    {
      return;
    }
    const Frame& frame = frames_[frame_index];
    SDL_FRect    dest{x, y, frame.rect.w * scale, frame.rect.h * scale};
    render_frame(renderer, frame_index, dest);
  }

private:
  void load_json(const std::filesystem::path& json_path)
  {
    std::ifstream file(json_path);
    if (!file.is_open())
    {
      throw std::runtime_error("Failed to open JSON file: " +
                               json_path.string());
    }

    nlohmann::json j;
    file >> j;

    if (j.contains("meta") && j["meta"].contains("image"))
    {
      image_filename_ = j["meta"]["image"].get<std::string>();
    }
    else
    {
      throw std::runtime_error("JSON missing meta.image field");
    }

    if (j.contains("frames"))
    {
      const auto& frames_json = j["frames"];
      if (frames_json.is_array())
      {
        for (const auto& frame_json : frames_json)
        {
          Frame       frame;
          frame.name            = frame_json.value("filename", "");
          const auto& rect_json = frame_json["frame"];
          frame.rect.x          = rect_json["x"].get<float>();
          frame.rect.y          = rect_json["y"].get<float>();
          frame.rect.w          = rect_json["w"].get<float>();
          frame.rect.h          = rect_json["h"].get<float>();
          frame.duration        = frame_json.value("duration", 100);
          frames_.push_back(frame);
        }
      }
      else if (frames_json.is_object())
      {
        for (const auto& [name, frame_json] : frames_json.items())
        {
          Frame       frame;
          frame.name            = name;
          const auto& rect_json = frame_json["frame"];
          frame.rect.x          = rect_json["x"].get<float>();
          frame.rect.y          = rect_json["y"].get<float>();
          frame.rect.w          = rect_json["w"].get<float>();
          frame.rect.h          = rect_json["h"].get<float>();
          frame.duration        = frame_json.value("duration", 100);
          frames_.push_back(frame);
        }
      }
    }

    if (j.contains("meta") && j["meta"].contains("frameTags"))
    {
      for (const auto& tag_json : j["meta"]["frameTags"])
      {
        FrameTag tag;
        tag.name      = tag_json["name"].get<std::string>();
        tag.from      = tag_json["from"].get<int>();
        tag.to        = tag_json["to"].get<int>();
        tag.direction = tag_json.value("direction", "forward");
        frame_tags_.push_back(tag);
      }
    }
  }

  void load_texture(SDL_Renderer*                renderer,
                    const std::filesystem::path& image_path)
  {
    SurfacePtr surface{SDL_LoadPNG(image_path.string().c_str()),
                       SDL_DestroySurface};
    if (!surface)
    {
      throw std::runtime_error("Failed to load PNG: " + image_path.string() +
                               " - " + SDL_GetError());
    }

    texture_.reset(SDL_CreateTextureFromSurface(renderer, surface.get()));
    if (!texture_)
    {
      throw std::runtime_error("Failed to create texture: " +
                               std::string(SDL_GetError()));
    }
  }

  TexturePtr            texture_;
  std::vector<Frame>    frames_;
  std::vector<FrameTag> frame_tags_;
  std::string           image_filename_;
};


#endif // _SDL3_SPRITE_SHEET_HPP_
