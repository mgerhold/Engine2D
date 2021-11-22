namespace ParticleSystemImpl {

    template<typename T>
    struct Range {
        T min;
        T max;
    };

    struct BezierCurve {
        glm::vec2 p0, p1;
        float minVal, maxVal;
        float leftY, rightY;
    };

    C2K_JSON_DEFINE_TYPE(BezierCurve, p0, p1, minVal, maxVal, leftY, rightY);

    struct RandomizedCurve {
        BezierCurve curve;
        BezierCurve randomness;
    };

    C2K_JSON_DEFINE_TYPE(RandomizedCurve, curve, randomness);

    struct BezierCurves2D {
        BezierCurve x;
        BezierCurve y;
    };

    C2K_JSON_DEFINE_TYPE(BezierCurves2D, x, y);

    struct RGBPivotPoint {
        float r;
        float g;
        float b;
    };

    C2K_JSON_DEFINE_TYPE(RGBPivotPoint, r, g, b);

    struct ColorGradientMark {
        c2k::Color color;
        float position;
    };

    C2K_JSON_DEFINE_TYPE(ColorGradientMark, color, position);

    struct ColorGradient {
        std::vector<ColorGradientMark> colorGradient;
    };

    C2K_JSON_DEFINE_TYPE(ColorGradient, colorGradient);

    struct ParticleBurst {
        double time;
        std::variant<float, Range<float>, BezierCurve, Range<BezierCurve>> count;
        std::optional<int> cycles;// otherwise infinity
        double interval;
        float probability;
    };

    C2K_JSON_DEFINE_TYPE(Range<float>, min, max);
    C2K_JSON_DEFINE_TYPE(Range<BezierCurve>, min, max);
    C2K_JSON_DEFINE_TYPE(ParticleBurst, time, count, cycles, interval, probability);

    struct ParticleSystemJSON {
        double duration;
        bool looping;
        bool prewarm;
        std::variant<double, Range<double>> startDelay;
        std::variant<double, Range<double>, BezierCurve, Range<BezierCurve>> startLifetime;
        std::variant<float, Range<float>, BezierCurve, Range<BezierCurve>> startSpeed;
        std::variant<glm::vec2, Range<glm::vec2>, BezierCurves2D, Range<BezierCurves2D>> startSize;
        std::variant<float, Range<float>, BezierCurve, Range<BezierCurve>> startRotation;
        float flipRotation;// 0.0f - 1.0f
        std::variant<Color, ColorGradient> color;
        std::variant<float, Range<float>, BezierCurve, Range<BezierCurve>> gravityModifier;
        bool simulateInWorldSpace;
        // maybe add: scaling mode? (local/hierarchy)
        std::uint32_t maxParticles;
        std::variant<float, Range<float>, BezierCurve, Range<BezierCurve>> rateOverTime;
        std::variant<float, Range<float>, BezierCurve, Range<BezierCurve>> rateOverDistance;
        std::optional<std::vector<ParticleBurst>> particleBursts;
        float emitterRadius;
        float emitterArc;// degrees
        std::variant<glm::vec2, Range<glm::vec2>, BezierCurves2D, Range<BezierCurves2D>> linearVelocityOverLifetime;
        std::variant<float, Range<float>, BezierCurve, Range<BezierCurve>> radialVelocityOverLifetime;
        std::variant<glm::vec2, Range<glm::vec2>, BezierCurves2D, Range<BezierCurves2D>> forceOverLifetime;
        std::optional<ColorGradient> colorOverLifetime;
        std::optional<BezierCurve> sizeOverLifetime;
        std::optional<BezierCurve> rotationOverLifetime;
        // TODO: sprite sheet animation settings?
    };

    C2K_JSON_DEFINE_TYPE(Range<double>, min, max);
    C2K_JSON_DEFINE_TYPE(Range<glm::vec2>, min, max);
    C2K_JSON_DEFINE_TYPE(Range<BezierCurves2D>, min, max);

    C2K_JSON_DEFINE_TYPE(ParticleSystemJSON,
                         duration,
                         looping,
                         prewarm,
                         startDelay,
                         startLifetime,
                         startSpeed,
                         startSize,
                         startRotation,
                         flipRotation,
                         color,
                         gravityModifier,
                         simulateInWorldSpace,
                         maxParticles,
                         rateOverTime,
                         rateOverDistance,
                         particleBursts,
                         emitterRadius,
                         emitterArc,
                         linearVelocityOverLifetime,
                         radialVelocityOverLifetime,
                         forceOverLifetime,
                         colorOverLifetime,
                         sizeOverLifetime,
                         rotationOverLifetime);

}// namespace ParticleSystemImpl