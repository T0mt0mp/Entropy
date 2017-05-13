/**
 * @file TextS.h
 * @author Tomas Polasek
 * @brief 
 */

#ifndef ECS_FIT_TEXTS_H
#define ECS_FIT_TEXTS_H

#include "Types.h"
#include "Font.h"

class TextS : public Universe::SystemT
{
public:
    using RequireT = ent::Require<PositionC>;

    /// Initialize the required members.
    TextS(ttf::Font *font);

    /**
     * Create all of the cubes.
     * @param size Number cubed will be the number of cubes.
     */
    void initializeCubes(u64 size);

    /**
     * Move the cubes into the default position.
     */
    void moveToDefault();

    /**
     * Move the cubes into a form of given text.
     * @param text The text to display.
     */
    void moveToText(const std::string &text);

    /// Advance to the next text.
    void nextText();

    /// If there was a text change, apply it.
    void updateText();

    /// Add new text to the rotation.
    void addText(const std::string &text);
private:
    f32 xStart() const;
    f32 yStart() const;
    f32 zStart() const;

    /// Calculate how many cubes can be used per unit of text.
    f64 cubesPerUnit(const std::string &text) const;

    /// Font used for text rendering.
    ttf::Font *mFont;

    /// Cube root of the number of cubes.
    u64 mSize;

    /// Current text index.
    u64 mTextIndex{0u};
    bool mTextChanged{false};

    /// List of available texts.
    std::vector<std::string> mTexts;

    /// On which z level will all the cubes be situated.
    static constexpr f32 Z_LEVEL{-20.0f};
    static constexpr f32 X_SPACE{1.0f};
    static constexpr f32 Y_SPACE{1.0f};
    static constexpr f32 Z_SPACE{1.0f};

    static constexpr f32 TEXT_OFFSET_MULT{-2.0f};
    static constexpr f32 TEXT_OFFSET{0.5f};
    static constexpr f32 TEXT_SCALE_FACTOR{450.0f};
    static constexpr f32 SPEED_MULT{2.0f};
    static constexpr f64 MIN_CUBES{4.0};
protected:
}; // class TextS

#endif //ECS_FIT_TEXTS_H
