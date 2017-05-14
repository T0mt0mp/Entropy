/**
 * @file TextS.cpp
 * @author Tomas Polasek
 * @brief 
 */

#include "TextS.h"

TextS::TextS(ttf::Font *font) :
    mFont{font}
{ }

f32 TextS::xStart() const
{ return -1.0f * ((mSize - 1) * X_SPACE / 2.0f); }
f32 TextS::yStart() const
{ return -1.0f * ((mSize - 1) * Y_SPACE / 2.0f); }
f32 TextS::zStart() const
{ return -1.0f * ((mSize - 1) * Z_SPACE / 2.0f) + Z_LEVEL; }

void TextS::initializeCubes(u64 size)
{
    mSize = size;
    const f32 X_START{xStart()};
    const f32 Y_START{yStart()};
    const f32 Z_START{zStart()};

    for (u64 zPos = 0; zPos < size; ++zPos)
    {
        for (u64 yPos = 0; yPos < size; ++yPos)
        {
            for (u64 xPos = 0; xPos < size; ++xPos)
            {
                Universe::EntityT e{universe()->createEntity()};
                e.add<PositionC>()->pos = glm::vec3(X_START + xPos * (X_SPACE),
                                                    Y_START + yPos * (Y_SPACE),
                                                    Z_START + zPos * (Z_SPACE));
                e.get<PositionC>()->rot = glm::vec3(0.0f);
                e.get<PositionC>()->scale = glm::vec3(0.2f);
                e.add<RotationVelocityC>()->rv = glm::vec3(glm::linearRand(-1.0f, 1.0f));
                e.add<TransformC>();
            }
        }
    }
}

void TextS::moveToDefault()
{
    const f32 X_START{xStart()};
    const f32 Y_START{yStart()};
    const f32 Z_START{zStart()};

    u64 xPos{0u};
    u64 yPos{0u};
    u64 zPos{0u};

    ASSERT_FAST(mSize * mSize * mSize == foreach().size());

    for (auto &e : foreach())
    {
        VelocityC *vel{e.add<VelocityC>()};
        PositionC *pos{e.get<PositionC>()};

        glm::vec3 targetPos{X_START + xPos * X_SPACE,
                            Y_START + yPos * Y_SPACE,
                            Z_START + zPos * Z_SPACE,
        };

        vel->targetPos = targetPos;
        //vel->vel = glm::normalize(targetPos - pos->pos) * 3.0f;
        vel->vel = (targetPos - pos->pos) / 3.0f;

        if (++xPos >= mSize)
        {
            if (++yPos >= mSize)
            {
                ++zPos;
                yPos = 0u;
            }

            xPos = 0u;
        }
    }
}

f64 TextS::cubesPerUnit(const std::string &text) const
{
    u64 numUnits{0u};
    u64 numLines{0u};

    for (char c: text)
    {
        if (c == '\0')
        {
            break;
        }
        else if (c == '\n')
        {
            continue;
        }

        auto letter = mFont->letter(c);
        auto it = letter.begin();
        auto end = letter.end();
        ASSERT_FAST(it != end);
        glm::vec2 last{it->x, it->y};
        //numLines++;
        ++it;
        for (; it != end; ++it)
        {
            if (it->newContour)
            {
                last = {it->x, it->y};
                continue;
            }

            numLines++;
            glm::vec2 current{it->x, it->y};

            numUnits += glm::length(current - last);
            last = current;
        }
    }

    return static_cast<f64>(numUnits) / (mSize * mSize * mSize - MIN_CUBES * numLines);
}

void TextS::moveToText(const std::string &text)
{
    if (text.empty())
    {
        return;
    }

    const u64 numCubes{mSize * mSize * mSize};
    const f64 upc{cubesPerUnit(text)};
    ASSERT_FAST(upc > 0.0);
    std::cout << upc << std::endl;

    glm::vec3 letterPos{text.size() * TEXT_OFFSET_MULT, Y_LEVEL, Z_LEVEL};

    ASSERT_FAST(mSize * mSize * mSize == foreach().size());

    auto eIt{foreach().begin()};
    auto eEnd{foreach().end()};
    f64 remainingCubes{0.0};

    for (char c : text)
    {
        ttf::FontLetter letter{mFont->letter(c)};
        ttf::FontLetterIterator lIt{letter.begin()};
        ttf::FontLetterIterator lEnd{letter.end()};
        glm::vec3 last{lIt->x, lIt->y, 0.0f};
        ++lIt;
        for (; lIt != lEnd; ++lIt)
        { // Iterate over all strokes.
            if (lIt->newContour)
            {
                last.x = lIt->x;
                last.y = lIt->y;
                continue;
            }
            glm::vec3 current{lIt->x, lIt->y, 0.0f};
            //std::cout << last.x << " " << last.y << " to " << current.x << " " << current.y << std::endl;
            f64 cubes{glm::length(glm::vec2(current.x - last.x, current.y - last.y)) / upc + remainingCubes};
            f64 fullCubes{0.0};
            remainingCubes = modf(cubes, &fullCubes);
            u64 realFullCubes{static_cast<u64>(fullCubes)};
            realFullCubes += MIN_CUBES;
            glm::vec3 delta{(current.x - last.x) / (realFullCubes - 1), (current.y - last.y) / (realFullCubes - 1), 0.0f};
            //std::cout << current.x / 102.0f << " " << current.y / 102.0f << std::endl;
            for (u64 iii = 0; iii < realFullCubes; ++iii)
            {
                VelocityC *vel{(*eIt).add<VelocityC>()};
                PositionC *pos{(*eIt).get<PositionC>()};

                glm::vec3 targetPos{last + static_cast<f32>(iii) * delta};
                targetPos /= TEXT_SCALE_FACTOR;
                targetPos += letterPos;
                //std::cout << targetPos.x << " " << targetPos.y << " " << targetPos.z << std::endl;
                vel->targetPos = targetPos;
                vel->vel = (targetPos - pos->pos) / SPEED_MULT;

                if (++eIt == eEnd)
                {
                    std::cout << "Force end" << std::endl;
                    return;
                    ASSERT_FATAL(false);
                }
            }
            last = current;
        }

        letterPos.x += letter.xMax() / TEXT_SCALE_FACTOR + TEXT_OFFSET;
    }
}

void TextS::nextText()
{
    if (!mTextChanged)
    {
        if (++mTextIndex > mTexts.size())
        {
            mTextIndex = 0u;
        }
        mTextChanged = true;
    }
}

void TextS::updateText()
{
    if (mTextChanged)
    {
        if (mTextIndex == 0u)
        {
            moveToDefault();
        }
        else
        {
            moveToText(mTexts[mTextIndex - 1u]);
        }
        mTextChanged = false;
    }
}

void TextS::addText(const std::string &text)
{
    mTexts.emplace_back(text);
}
