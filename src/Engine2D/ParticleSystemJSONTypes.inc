namespace ParticleSystemImpl {

    template<typename T>
    struct Range {
        T min;
        T max;
    };

    struct BezierPoint {
        glm::vec2 position; // x normalized, y absolute
        glm::vec2 direction;// unit vector
    };

    C2K_JSON_DEFINE_TYPE(BezierPoint, position, direction);

    struct BezierSpline {
        std::vector<BezierPoint> pivotPoints;
    };

    C2K_JSON_DEFINE_TYPE(BezierSpline, pivotPoints);

    struct BezierSplines2D {
        BezierSpline x;
        BezierSpline y;
    };

    C2K_JSON_DEFINE_TYPE(BezierSplines2D, x, y);
    C2K_JSON_DEFINE_TYPE(Color, r, g, b, a);

    struct RGBPivotPoint {
        float r;
        float g;
        float b;
    };

    C2K_JSON_DEFINE_TYPE(RGBPivotPoint, r, g, b);

    struct ColorGradient {
        std::vector<float> rgbInterpolationParameters;
        std::vector<RGBPivotPoint> rgbPivotPoints;
        std::vector<float> alphaInterpolationParameters;
        std::vector<float> alphaPivotValues;
    };

    C2K_JSON_DEFINE_TYPE(ColorGradient,
                         rgbInterpolationParameters,
                         rgbPivotPoints,
                         alphaInterpolationParameters,
                         alphaPivotValues);

    struct ParticleBurst {
        double time;
        std::variant<float, Range<float>, BezierSpline, Range<BezierSpline>> count;
        std::optional<int> cycles;// otherwise infinity
        double interval;
        float probability;
    };

    C2K_JSON_DEFINE_TYPE(Range<float>, min, max);
    C2K_JSON_DEFINE_TYPE(Range<BezierSpline>, min, max);
    C2K_JSON_DEFINE_TYPE(ParticleBurst, time, count, cycles, interval, probability);

    struct ParticleSystemJSON {
        double duration;
        bool looping;
        bool prewarm;
        std::variant<double, Range<double>> startDelay;
        std::variant<double, Range<double>, BezierSpline, Range<BezierSpline>> startLifetime;
        std::variant<float, Range<float>, BezierSpline, Range<BezierSpline>> startSpeed;
        std::variant<glm::vec2, Range<glm::vec2>, BezierSplines2D, Range<BezierSplines2D>> startSize;
        std::variant<float, Range<float>, BezierSpline, Range<BezierSpline>> startRotation;
        float flipRotation;// 0.0f - 1.0f
        std::variant<Color, ColorGradient> color;
        std::variant<float, Range<float>, BezierSpline, Range<BezierSpline>> gravityModifier;
        bool simulateInWorldSpace;
        // maybe add: scaling mode? (local/hierarchy)
        std::uint32_t maxParticles;
        std::variant<float, Range<float>, BezierSpline, Range<BezierSpline>> rateOverTime;
        std::variant<float, Range<float>, BezierSpline, Range<BezierSpline>> rateOverDistance;
        std::optional<std::vector<ParticleBurst>> particleBursts;
        float emitterRadius;
        float emitterArc;// degrees
        std::variant<glm::vec2, Range<glm::vec2>, BezierSplines2D, Range<BezierSplines2D>> linearVelocityOverLifetime;
        std::variant<float, Range<float>, BezierSpline, Range<BezierSpline>> radialVelocityOverLifetime;
        std::variant<glm::vec2, Range<glm::vec2>, BezierSplines2D, Range<BezierSplines2D>> forceOverLifetime;
        std::optional<ColorGradient> colorOverLifetime;
        std::optional<BezierSpline> sizeOverLifetime;
        std::optional<BezierSpline> rotationOverLifetime;
        // TODO: sprite sheet animation settings?
    };

    C2K_JSON_DEFINE_TYPE(Range<double>, min, max);
    C2K_JSON_DEFINE_TYPE(Range<glm::vec2>, min, max);
    C2K_JSON_DEFINE_TYPE(Range<BezierSplines2D>, min, max);

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