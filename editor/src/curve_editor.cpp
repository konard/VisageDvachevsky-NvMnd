/**
 * @file curve_editor.cpp
 * @brief Implementation of Animation Curve Editor for NovelMind
 */

#include "NovelMind/editor/curve_editor.hpp"
#include "NovelMind/core/logger.hpp"
#include <algorithm>
#include <cmath>
#include <sstream>

namespace NovelMind::editor {

// =============================================================================
// AnimationCurve Implementation
// =============================================================================

AnimationCurve::AnimationCurve()
    : m_id(std::to_string(reinterpret_cast<uintptr_t>(this))),
      m_name("Unnamed Curve") {
  // Add default start and end points
  CurvePoint start;
  start.time = 0.0f;
  start.value = 0.0f;
  m_points.push_back(start);

  CurvePoint end;
  end.time = 1.0f;
  end.value = 1.0f;
  m_points.push_back(end);
}

AnimationCurve::AnimationCurve(const std::string &name)
    : m_id(std::to_string(reinterpret_cast<uintptr_t>(this))), m_name(name) {
  CurvePoint start;
  start.time = 0.0f;
  start.value = 0.0f;
  m_points.push_back(start);

  CurvePoint end;
  end.time = 1.0f;
  end.value = 1.0f;
  m_points.push_back(end);
}

void AnimationCurve::addPoint(const CurvePoint &point) {
  // Insert in sorted order by time
  auto it = std::lower_bound(
      m_points.begin(), m_points.end(), point,
      [](const CurvePoint &a, const CurvePoint &b) { return a.time < b.time; });
  m_points.insert(it, point);
}

void AnimationCurve::removePoint(size_t index) {
  if (index < m_points.size() && m_points.size() > 2) {
    m_points.erase(m_points.begin() + static_cast<std::ptrdiff_t>(index));
  }
}

void AnimationCurve::updatePoint(size_t index, const CurvePoint &point) {
  if (index < m_points.size()) {
    m_points[index] = point;
    // Re-sort if necessary
    std::sort(m_points.begin(), m_points.end(),
              [](const CurvePoint &a, const CurvePoint &b) {
                return a.time < b.time;
              });
  }
}

f32 AnimationCurve::evaluate(f32 t) const {
  if (m_points.empty())
    return 0.0f;
  if (m_points.size() == 1)
    return m_points[0].value;

  t = std::clamp(t, 0.0f, 1.0f);

  // Find the segment
  i32 segIndex = findSegment(t);
  if (segIndex < 0)
    return m_points[0].value;
  if (segIndex >= static_cast<i32>(m_points.size()) - 1) {
    return m_points.back().value;
  }

  const auto &p0 = m_points[static_cast<size_t>(segIndex)];
  const auto &p1 = m_points[static_cast<size_t>(segIndex + 1)];

  // Calculate local t for this segment
  f32 segmentLength = p1.time - p0.time;
  if (segmentLength < 0.0001f)
    return p0.value;

  f32 localT = (t - p0.time) / segmentLength;

  return evaluateBezierSegment(p0, p1, localT);
}

f32 AnimationCurve::evaluateDerivative(f32 t) const {
  // Numerical derivative
  const f32 epsilon = 0.0001f;
  f32 t0 = std::max(0.0f, t - epsilon);
  f32 t1 = std::min(1.0f, t + epsilon);
  return (evaluate(t1) - evaluate(t0)) / (t1 - t0);
}

std::vector<renderer::Vec2> AnimationCurve::sample(i32 numSamples) const {
  std::vector<renderer::Vec2> samples;
  samples.reserve(static_cast<size_t>(numSamples));

  for (i32 i = 0; i < numSamples; ++i) {
    f32 t = static_cast<f32>(i) / static_cast<f32>(numSamples - 1);
    samples.push_back({t, evaluate(t)});
  }

  return samples;
}

AnimationCurve AnimationCurve::createLinear() {
  AnimationCurve curve("Linear");
  curve.m_points.clear();

  CurvePoint start;
  start.time = 0.0f;
  start.value = 0.0f;
  start.handleMode = CurvePoint::HandleMode::Vector;
  curve.m_points.push_back(start);

  CurvePoint end;
  end.time = 1.0f;
  end.value = 1.0f;
  end.handleMode = CurvePoint::HandleMode::Vector;
  curve.m_points.push_back(end);

  return curve;
}

AnimationCurve AnimationCurve::createEaseIn() {
  AnimationCurve curve("Ease In");
  curve.m_points.clear();

  CurvePoint start;
  start.time = 0.0f;
  start.value = 0.0f;
  start.outHandleX = 0.42f;
  start.outHandleY = 0.0f;
  curve.m_points.push_back(start);

  CurvePoint end;
  end.time = 1.0f;
  end.value = 1.0f;
  end.inHandleX = -0.42f;
  end.inHandleY = 0.0f;
  curve.m_points.push_back(end);

  return curve;
}

AnimationCurve AnimationCurve::createEaseOut() {
  AnimationCurve curve("Ease Out");
  curve.m_points.clear();

  CurvePoint start;
  start.time = 0.0f;
  start.value = 0.0f;
  start.outHandleX = 0.0f;
  start.outHandleY = 0.42f;
  curve.m_points.push_back(start);

  CurvePoint end;
  end.time = 1.0f;
  end.value = 1.0f;
  end.inHandleX = -0.58f;
  end.inHandleY = 0.0f;
  curve.m_points.push_back(end);

  return curve;
}

AnimationCurve AnimationCurve::createEaseInOut() {
  AnimationCurve curve("Ease In Out");
  curve.m_points.clear();

  CurvePoint start;
  start.time = 0.0f;
  start.value = 0.0f;
  start.outHandleX = 0.42f;
  start.outHandleY = 0.0f;
  curve.m_points.push_back(start);

  CurvePoint end;
  end.time = 1.0f;
  end.value = 1.0f;
  end.inHandleX = -0.42f;
  end.inHandleY = 0.0f;
  curve.m_points.push_back(end);

  return curve;
}

AnimationCurve AnimationCurve::createEaseInQuad() {
  AnimationCurve curve("Ease In Quad");
  curve.m_points.clear();

  CurvePoint start;
  start.time = 0.0f;
  start.value = 0.0f;
  start.outHandleX = 0.55f;
  start.outHandleY = 0.0f;
  curve.m_points.push_back(start);

  CurvePoint end;
  end.time = 1.0f;
  end.value = 1.0f;
  end.inHandleX = -0.15f;
  end.inHandleY = 0.0f;
  curve.m_points.push_back(end);

  return curve;
}

AnimationCurve AnimationCurve::createEaseOutQuad() {
  AnimationCurve curve("Ease Out Quad");
  curve.m_points.clear();

  CurvePoint start;
  start.time = 0.0f;
  start.value = 0.0f;
  start.outHandleX = 0.25f;
  start.outHandleY = 0.46f;
  curve.m_points.push_back(start);

  CurvePoint end;
  end.time = 1.0f;
  end.value = 1.0f;
  end.inHandleX = -0.25f;
  end.inHandleY = 0.0f;
  curve.m_points.push_back(end);

  return curve;
}

AnimationCurve AnimationCurve::createEaseInOutQuad() {
  AnimationCurve curve("Ease In Out Quad");
  curve.m_points.clear();

  CurvePoint start;
  start.time = 0.0f;
  start.value = 0.0f;
  start.outHandleX = 0.46f;
  start.outHandleY = 0.03f;
  curve.m_points.push_back(start);

  CurvePoint end;
  end.time = 1.0f;
  end.value = 1.0f;
  end.inHandleX = -0.46f;
  end.inHandleY = 0.03f;
  curve.m_points.push_back(end);

  return curve;
}

AnimationCurve AnimationCurve::createEaseInCubic() {
  AnimationCurve curve("Ease In Cubic");
  curve.m_points.clear();

  CurvePoint start;
  start.time = 0.0f;
  start.value = 0.0f;
  start.outHandleX = 0.55f;
  start.outHandleY = 0.0f;
  curve.m_points.push_back(start);

  CurvePoint end;
  end.time = 1.0f;
  end.value = 1.0f;
  end.inHandleX = -0.325f;
  end.inHandleY = 0.0f;
  curve.m_points.push_back(end);

  return curve;
}

AnimationCurve AnimationCurve::createEaseOutCubic() {
  AnimationCurve curve("Ease Out Cubic");
  curve.m_points.clear();

  CurvePoint start;
  start.time = 0.0f;
  start.value = 0.0f;
  start.outHandleX = 0.215f;
  start.outHandleY = 0.61f;
  curve.m_points.push_back(start);

  CurvePoint end;
  end.time = 1.0f;
  end.value = 1.0f;
  end.inHandleX = -0.145f;
  end.inHandleY = 0.0f;
  curve.m_points.push_back(end);

  return curve;
}

AnimationCurve AnimationCurve::createEaseInOutCubic() {
  AnimationCurve curve("Ease In Out Cubic");
  curve.m_points.clear();

  CurvePoint start;
  start.time = 0.0f;
  start.value = 0.0f;
  start.outHandleX = 0.645f;
  start.outHandleY = 0.045f;
  curve.m_points.push_back(start);

  CurvePoint end;
  end.time = 1.0f;
  end.value = 1.0f;
  end.inHandleX = -0.645f;
  end.inHandleY = 0.045f;
  curve.m_points.push_back(end);

  return curve;
}

AnimationCurve AnimationCurve::createEaseInBack() {
  AnimationCurve curve("Ease In Back");
  curve.m_points.clear();

  CurvePoint start;
  start.time = 0.0f;
  start.value = 0.0f;
  start.outHandleX = 0.36f;
  start.outHandleY = 0.0f;
  curve.m_points.push_back(start);

  CurvePoint end;
  end.time = 1.0f;
  end.value = 1.0f;
  end.inHandleX = -0.45f;
  end.inHandleY = -0.29f;
  curve.m_points.push_back(end);

  return curve;
}

AnimationCurve AnimationCurve::createEaseOutBack() {
  AnimationCurve curve("Ease Out Back");
  curve.m_points.clear();

  CurvePoint start;
  start.time = 0.0f;
  start.value = 0.0f;
  start.outHandleX = 0.175f;
  start.outHandleY = 0.885f;
  curve.m_points.push_back(start);

  CurvePoint end;
  end.time = 1.0f;
  end.value = 1.0f;
  end.inHandleX = -0.32f;
  end.inHandleY = 0.0f;
  curve.m_points.push_back(end);

  return curve;
}

AnimationCurve AnimationCurve::createEaseInOutBack() {
  AnimationCurve curve("Ease In Out Back");
  curve.m_points.clear();

  CurvePoint start;
  start.time = 0.0f;
  start.value = 0.0f;
  start.outHandleX = 0.68f;
  start.outHandleY = -0.55f;
  curve.m_points.push_back(start);

  CurvePoint end;
  end.time = 1.0f;
  end.value = 1.0f;
  end.inHandleX = -0.68f;
  end.inHandleY = 0.55f;
  curve.m_points.push_back(end);

  return curve;
}

AnimationCurve AnimationCurve::createEaseInBounce() {
  AnimationCurve curve("Ease In Bounce");
  // Bounce curves need multiple points
  curve.m_points.clear();

  CurvePoint p0;
  p0.time = 0.0f;
  p0.value = 0.0f;
  curve.m_points.push_back(p0);

  CurvePoint p1;
  p1.time = 0.09f;
  p1.value = 0.03f;
  curve.m_points.push_back(p1);

  CurvePoint p2;
  p2.time = 0.27f;
  p2.value = 0.06f;
  curve.m_points.push_back(p2);

  CurvePoint p3;
  p3.time = 0.55f;
  p3.value = 0.23f;
  curve.m_points.push_back(p3);

  CurvePoint p4;
  p4.time = 1.0f;
  p4.value = 1.0f;
  curve.m_points.push_back(p4);

  return curve;
}

AnimationCurve AnimationCurve::createEaseOutBounce() {
  AnimationCurve curve("Ease Out Bounce");
  curve.m_points.clear();

  CurvePoint p0;
  p0.time = 0.0f;
  p0.value = 0.0f;
  curve.m_points.push_back(p0);

  CurvePoint p1;
  p1.time = 0.36f;
  p1.value = 0.77f;
  curve.m_points.push_back(p1);

  CurvePoint p2;
  p2.time = 0.73f;
  p2.value = 0.94f;
  curve.m_points.push_back(p2);

  CurvePoint p3;
  p3.time = 0.91f;
  p3.value = 0.97f;
  curve.m_points.push_back(p3);

  CurvePoint p4;
  p4.time = 1.0f;
  p4.value = 1.0f;
  curve.m_points.push_back(p4);

  return curve;
}

AnimationCurve AnimationCurve::createEaseInOutBounce() {
  AnimationCurve curve("Ease In Out Bounce");
  curve.m_points.clear();

  CurvePoint p0;
  p0.time = 0.0f;
  p0.value = 0.0f;
  curve.m_points.push_back(p0);

  CurvePoint p1;
  p1.time = 0.15f;
  p1.value = 0.03f;
  curve.m_points.push_back(p1);

  CurvePoint p2;
  p2.time = 0.5f;
  p2.value = 0.5f;
  curve.m_points.push_back(p2);

  CurvePoint p3;
  p3.time = 0.85f;
  p3.value = 0.97f;
  curve.m_points.push_back(p3);

  CurvePoint p4;
  p4.time = 1.0f;
  p4.value = 1.0f;
  curve.m_points.push_back(p4);

  return curve;
}

AnimationCurve AnimationCurve::createEaseInElastic() {
  AnimationCurve curve("Ease In Elastic");
  curve.m_points.clear();

  CurvePoint p0;
  p0.time = 0.0f;
  p0.value = 0.0f;
  curve.m_points.push_back(p0);

  CurvePoint p1;
  p1.time = 0.4f;
  p1.value = -0.01f;
  curve.m_points.push_back(p1);

  CurvePoint p2;
  p2.time = 0.7f;
  p2.value = 0.02f;
  curve.m_points.push_back(p2);

  CurvePoint p3;
  p3.time = 0.85f;
  p3.value = -0.06f;
  curve.m_points.push_back(p3);

  CurvePoint p4;
  p4.time = 1.0f;
  p4.value = 1.0f;
  curve.m_points.push_back(p4);

  return curve;
}

AnimationCurve AnimationCurve::createEaseOutElastic() {
  AnimationCurve curve("Ease Out Elastic");
  curve.m_points.clear();

  CurvePoint p0;
  p0.time = 0.0f;
  p0.value = 0.0f;
  curve.m_points.push_back(p0);

  CurvePoint p1;
  p1.time = 0.15f;
  p1.value = 1.06f;
  curve.m_points.push_back(p1);

  CurvePoint p2;
  p2.time = 0.3f;
  p2.value = 0.98f;
  curve.m_points.push_back(p2);

  CurvePoint p3;
  p3.time = 0.6f;
  p3.value = 1.01f;
  curve.m_points.push_back(p3);

  CurvePoint p4;
  p4.time = 1.0f;
  p4.value = 1.0f;
  curve.m_points.push_back(p4);

  return curve;
}

AnimationCurve AnimationCurve::createEaseInOutElastic() {
  AnimationCurve curve("Ease In Out Elastic");
  curve.m_points.clear();

  CurvePoint p0;
  p0.time = 0.0f;
  p0.value = 0.0f;
  curve.m_points.push_back(p0);

  CurvePoint p1;
  p1.time = 0.35f;
  p1.value = 0.0f;
  curve.m_points.push_back(p1);

  CurvePoint p2;
  p2.time = 0.45f;
  p2.value = -0.03f;
  curve.m_points.push_back(p2);

  CurvePoint p3;
  p3.time = 0.5f;
  p3.value = 0.5f;
  curve.m_points.push_back(p3);

  CurvePoint p4;
  p4.time = 0.55f;
  p4.value = 1.03f;
  curve.m_points.push_back(p4);

  CurvePoint p5;
  p5.time = 0.65f;
  p5.value = 1.0f;
  curve.m_points.push_back(p5);

  CurvePoint p6;
  p6.time = 1.0f;
  p6.value = 1.0f;
  curve.m_points.push_back(p6);

  return curve;
}

AnimationCurve AnimationCurve::createStep() {
  AnimationCurve curve("Step");
  curve.m_points.clear();

  CurvePoint start;
  start.time = 0.0f;
  start.value = 0.0f;
  start.handleMode = CurvePoint::HandleMode::Broken;
  curve.m_points.push_back(start);

  CurvePoint mid;
  mid.time = 0.5f;
  mid.value = 1.0f;
  mid.handleMode = CurvePoint::HandleMode::Broken;
  curve.m_points.push_back(mid);

  CurvePoint end;
  end.time = 1.0f;
  end.value = 1.0f;
  end.handleMode = CurvePoint::HandleMode::Broken;
  curve.m_points.push_back(end);

  return curve;
}

AnimationCurve AnimationCurve::createSmooth() {
  AnimationCurve curve("Smooth");
  curve.m_points.clear();

  CurvePoint start;
  start.time = 0.0f;
  start.value = 0.0f;
  start.outHandleX = 0.25f;
  start.outHandleY = 0.1f;
  curve.m_points.push_back(start);

  CurvePoint end;
  end.time = 1.0f;
  end.value = 1.0f;
  end.inHandleX = -0.25f;
  end.inHandleY = -0.1f;
  curve.m_points.push_back(end);

  return curve;
}

bool AnimationCurve::isValid() const {
  if (m_points.size() < 2)
    return false;

  // Check that points are sorted
  for (size_t i = 1; i < m_points.size(); ++i) {
    if (m_points[i].time < m_points[i - 1].time)
      return false;
  }

  // Check range
  if (m_points.front().time < 0.0f || m_points.back().time > 1.0f)
    return false;

  return true;
}

void AnimationCurve::normalize() {
  // Sort by time
  std::sort(
      m_points.begin(), m_points.end(),
      [](const CurvePoint &a, const CurvePoint &b) { return a.time < b.time; });

  // Clamp values
  for (auto &point : m_points) {
    point.time = std::clamp(point.time, 0.0f, 1.0f);
  }
}

Result<void> AnimationCurve::save(const std::string &path) const {
  (void)path;
  return Result<void>::ok();
}

Result<AnimationCurve> AnimationCurve::load(const std::string &path) {
  (void)path;
  return Result<AnimationCurve>::ok(AnimationCurve());
}

std::string AnimationCurve::toJson() const {
  std::ostringstream json;
  json << "{\"name\":\"" << m_name << "\",\"points\":[";
  for (size_t i = 0; i < m_points.size(); ++i) {
    const auto &p = m_points[i];
    if (i > 0)
      json << ",";
    json << "{\"t\":" << p.time << ",\"v\":" << p.value << "}";
  }
  json << "]}";
  return json.str();
}

Result<AnimationCurve> AnimationCurve::fromJson(const std::string &json) {
  (void)json;
  return Result<AnimationCurve>::ok(AnimationCurve());
}

f32 AnimationCurve::evaluateBezierSegment(const CurvePoint &p0,
                                          const CurvePoint &p1,
                                          f32 localT) const {
  // Cubic bezier interpolation
  f32 t2 = localT * localT;
  f32 t3 = t2 * localT;
  f32 mt = 1.0f - localT;
  f32 mt2 = mt * mt;
  f32 mt3 = mt2 * mt;

  // Control points
  f32 cp0 = p0.value;
  f32 cp1 = p0.value + p0.outHandleY;
  f32 cp2 = p1.value + p1.inHandleY;
  f32 cp3 = p1.value;

  return mt3 * cp0 + 3.0f * mt2 * localT * cp1 + 3.0f * mt * t2 * cp2 +
         t3 * cp3;
}

i32 AnimationCurve::findSegment(f32 t) const {
  for (size_t i = 0; i < m_points.size() - 1; ++i) {
    if (t >= m_points[i].time && t <= m_points[i + 1].time) {
      return static_cast<i32>(i);
    }
  }
  return static_cast<i32>(m_points.size()) - 2;
}

// =============================================================================
// CurveLibrary Implementation
// =============================================================================

CurveLibrary::CurveLibrary() { loadPresets(); }

void CurveLibrary::addCurve(const AnimationCurve &curve) {
  m_curves[curve.getId()] = curve;
}

void CurveLibrary::removeCurve(const std::string &id) { m_curves.erase(id); }

void CurveLibrary::updateCurve(const std::string &id,
                               const AnimationCurve &curve) {
  auto it = m_curves.find(id);
  if (it != m_curves.end()) {
    it->second = curve;
  }
}

const AnimationCurve *CurveLibrary::getCurve(const std::string &id) const {
  auto it = m_curves.find(id);
  if (it != m_curves.end()) {
    return &it->second;
  }
  return nullptr;
}

std::vector<std::string> CurveLibrary::getCurveIds() const {
  std::vector<std::string> ids;
  ids.reserve(m_curves.size());
  for (const auto &[id, curve] : m_curves) {
    ids.push_back(id);
  }
  return ids;
}

std::vector<std::string> CurveLibrary::getCurveNames() const {
  std::vector<std::string> names;
  names.reserve(m_curves.size());
  for (const auto &[id, curve] : m_curves) {
    names.push_back(curve.getName());
  }
  return names;
}

void CurveLibrary::setCurveCategory(const std::string &curveId,
                                    const std::string &category) {
  m_curveCategories[curveId] = category;
}

std::vector<std::string>
CurveLibrary::getCurvesInCategory(const std::string &category) const {
  std::vector<std::string> result;
  for (const auto &[id, cat] : m_curveCategories) {
    if (cat == category) {
      result.push_back(id);
    }
  }
  return result;
}

std::vector<std::string> CurveLibrary::getCategories() const {
  std::vector<std::string> categories;
  for (const auto &[id, cat] : m_curveCategories) {
    if (std::find(categories.begin(), categories.end(), cat) ==
        categories.end()) {
      categories.push_back(cat);
    }
  }
  return categories;
}

void CurveLibrary::loadPresets() {
  // Add all preset curves
  auto addPreset = [this](const AnimationCurve &curve,
                          const std::string &category) {
    m_curves[curve.getId()] = curve;
    m_curveCategories[curve.getId()] = category;
    m_presetIds.push_back(curve.getId());
  };

  addPreset(AnimationCurve::createLinear(), "Basic");
  addPreset(AnimationCurve::createEaseIn(), "Basic");
  addPreset(AnimationCurve::createEaseOut(), "Basic");
  addPreset(AnimationCurve::createEaseInOut(), "Basic");

  addPreset(AnimationCurve::createEaseInQuad(), "Quadratic");
  addPreset(AnimationCurve::createEaseOutQuad(), "Quadratic");
  addPreset(AnimationCurve::createEaseInOutQuad(), "Quadratic");

  addPreset(AnimationCurve::createEaseInCubic(), "Cubic");
  addPreset(AnimationCurve::createEaseOutCubic(), "Cubic");
  addPreset(AnimationCurve::createEaseInOutCubic(), "Cubic");

  addPreset(AnimationCurve::createEaseInBack(), "Back");
  addPreset(AnimationCurve::createEaseOutBack(), "Back");
  addPreset(AnimationCurve::createEaseInOutBack(), "Back");

  addPreset(AnimationCurve::createEaseInBounce(), "Bounce");
  addPreset(AnimationCurve::createEaseOutBounce(), "Bounce");
  addPreset(AnimationCurve::createEaseInOutBounce(), "Bounce");

  addPreset(AnimationCurve::createEaseInElastic(), "Elastic");
  addPreset(AnimationCurve::createEaseOutElastic(), "Elastic");
  addPreset(AnimationCurve::createEaseInOutElastic(), "Elastic");

  addPreset(AnimationCurve::createStep(), "Special");
  addPreset(AnimationCurve::createSmooth(), "Special");
}

std::vector<std::string> CurveLibrary::getPresetIds() const {
  return m_presetIds;
}

Result<void> CurveLibrary::saveLibrary(const std::string &path) {
  (void)path;
  return Result<void>::ok();
}

Result<void> CurveLibrary::loadLibrary(const std::string &path) {
  (void)path;
  return Result<void>::ok();
}

// =============================================================================
// CurveEditor Implementation
// =============================================================================

CurveEditor::CurveEditor() {}

CurveEditor::~CurveEditor() = default;

void CurveEditor::update(f64 deltaTime) {
  if (m_previewPlaying) {
    m_previewTime += static_cast<f32>(deltaTime);
    if (m_previewTime > 2.0f) {
      m_previewTime = 0.0f;
    }
  }

  handleInput();
}

void CurveEditor::render() {
  // isVisible() check already handled in class definition
  if (!isVisible())
    return;

  renderGrid();
  renderCurve();
  renderPoints();
  renderHandles();

  if (m_config.showPreview) {
    renderPreview();
  }

  if (m_showPresetsPanel) {
    renderPresetsPanel();
  }
}

void CurveEditor::onResize(i32 width, i32 height) {
  m_width = width;
  m_height = height;
}

void CurveEditor::setCurve(AnimationCurve *curve) {
  m_curve = curve;
  m_selectedPoints.clear();
}

void CurveEditor::selectPoint(size_t index) {
  if (std::find(m_selectedPoints.begin(), m_selectedPoints.end(), index) ==
      m_selectedPoints.end()) {
    m_selectedPoints.push_back(index);
  }

  if (m_onPointSelected) {
    m_onPointSelected(index);
  }
}

void CurveEditor::deselectAll() { m_selectedPoints.clear(); }

void CurveEditor::setZoom(f32 zoom) {
  m_zoom = std::clamp(zoom, m_config.minZoom, m_config.maxZoom);
}

void CurveEditor::setPan(f32 x, f32 y) {
  m_panX = x;
  m_panY = y;
}

void CurveEditor::fitToView() {
  m_zoom = 1.0f;
  m_panX = 0.0f;
  m_panY = 0.0f;
}

void CurveEditor::resetView() { fitToView(); }

void CurveEditor::setConfig(const CurveEditorConfig &config) {
  m_config = config;
}

void CurveEditor::showPresetsPanel() { m_showPresetsPanel = true; }

void CurveEditor::hidePresetsPanel() { m_showPresetsPanel = false; }

void CurveEditor::applyPreset(const std::string &presetId) {
  if (!m_library || !m_curve)
    return;

  const auto *preset = m_library->getCurve(presetId);
  if (preset) {
    *m_curve = *preset;
    if (m_onCurveModified) {
      m_onCurveModified(*m_curve);
    }
  }
}

void CurveEditor::setOnCurveModified(
    std::function<void(const AnimationCurve &)> callback) {
  m_onCurveModified = std::move(callback);
}

void CurveEditor::setOnPointSelected(std::function<void(size_t)> callback) {
  m_onPointSelected = std::move(callback);
}

void CurveEditor::renderGrid() {
  // Grid rendering implementation
}

void CurveEditor::renderCurve() {
  if (!m_curve)
    return;
  // Curve rendering implementation
}

void CurveEditor::renderPoints() {
  if (!m_curve)
    return;
  // Point rendering implementation
}

void CurveEditor::renderHandles() {
  if (!m_curve)
    return;
  // Handle rendering implementation
}

void CurveEditor::renderPreview() {
  // Preview animation rendering
}

void CurveEditor::renderPresetsPanel() {
  // Presets panel rendering
}

void CurveEditor::handleInput() {
  handlePan();
  handleZoom();
  handlePointDrag();
  handleHandleDrag();
}

void CurveEditor::handlePointDrag() {
  // Point drag implementation
}

void CurveEditor::handleHandleDrag() {
  // Handle drag implementation
}

void CurveEditor::handlePan() {
  // Pan handling implementation
}

void CurveEditor::handleZoom() {
  // Zoom handling implementation
}

renderer::Vec2 CurveEditor::curveToScreen(f32 t, f32 v) const {
  f32 x = (t - m_panX) * m_zoom * static_cast<f32>(m_width);
  f32 y = static_cast<f32>(m_height) -
          (v - m_panY) * m_zoom * static_cast<f32>(m_height);
  return {x, y};
}

renderer::Vec2 CurveEditor::screenToCurve(f32 x, f32 y) const {
  f32 t = x / (m_zoom * static_cast<f32>(m_width)) + m_panX;
  f32 v =
      (static_cast<f32>(m_height) - y) / (m_zoom * static_cast<f32>(m_height)) +
      m_panY;
  return {t, v};
}

i32 CurveEditor::hitTestPoint(f32 x, f32 y) const {
  if (!m_curve)
    return -1;

  const auto &points = m_curve->getPoints();
  for (size_t i = 0; i < points.size(); ++i) {
    renderer::Vec2 screenPos = curveToScreen(points[i].time, points[i].value);
    f32 dx = x - screenPos.x;
    f32 dy = y - screenPos.y;
    if (dx * dx + dy * dy <= m_config.pointRadius * m_config.pointRadius) {
      return static_cast<i32>(i);
    }
  }
  return -1;
}

i32 CurveEditor::hitTestHandle(f32 x, f32 y, bool &isInHandle) const {
  (void)x;
  (void)y;
  isInHandle = false;
  return -1;
}

void CurveEditor::addPointAtScreenPos(f32 x, f32 y) {
  if (!m_curve)
    return;

  renderer::Vec2 curvePos = screenToCurve(x, y);

  CurvePoint point;
  point.time = curvePos.x;
  point.value = curvePos.y;
  m_curve->addPoint(point);

  if (m_onCurveModified) {
    m_onCurveModified(*m_curve);
  }
}

void CurveEditor::deleteSelectedPoints() {
  if (!m_curve)
    return;

  // Sort in reverse to delete from end
  std::sort(m_selectedPoints.rbegin(), m_selectedPoints.rend());

  for (size_t index : m_selectedPoints) {
    m_curve->removePoint(index);
  }

  m_selectedPoints.clear();

  if (m_onCurveModified) {
    m_onCurveModified(*m_curve);
  }
}

void CurveEditor::updatePointPosition(size_t index, f32 t, f32 v) {
  if (!m_curve)
    return;

  auto points = m_curve->getPoints();
  if (index >= points.size())
    return;

  CurvePoint point = points[index];
  point.time = t;
  point.value = v;
  m_curve->updatePoint(index, point);

  if (m_onCurveModified) {
    m_onCurveModified(*m_curve);
  }
}

void CurveEditor::updateHandlePosition(size_t index, bool isInHandle, f32 dx,
                                       f32 dy) {
  if (!m_curve)
    return;

  auto points = m_curve->getPoints();
  if (index >= points.size())
    return;

  CurvePoint point = points[index];
  if (isInHandle) {
    point.inHandleX += dx;
    point.inHandleY += dy;
  } else {
    point.outHandleX += dx;
    point.outHandleY += dy;
  }

  m_curve->updatePoint(index, point);

  if (m_onCurveModified) {
    m_onCurveModified(*m_curve);
  }
}

// =============================================================================
// InlineCurveWidget Implementation
// =============================================================================

InlineCurveWidget::InlineCurveWidget(f32 width, f32 height)
    : m_width(width), m_height(height) {}

void InlineCurveWidget::setCurve(const AnimationCurve &curve) {
  m_curve = curve;
}

void InlineCurveWidget::setSize(f32 width, f32 height) {
  m_width = width;
  m_height = height;
}

void InlineCurveWidget::update(f64 deltaTime) { (void)deltaTime; }

void InlineCurveWidget::render(renderer::IRenderer *renderer, f32 x, f32 y) {
  (void)renderer;
  (void)x;
  (void)y;
  // Inline widget rendering implementation
}

bool InlineCurveWidget::handleClick(f32 x, f32 y) {
  (void)x;
  (void)y;
  return false;
}

bool InlineCurveWidget::handleDrag(f32 x, f32 y, f32 dx, f32 dy) {
  (void)x;
  (void)y;
  (void)dx;
  (void)dy;
  return false;
}

void InlineCurveWidget::handleRelease() {
  m_isDragging = false;
  m_selectedPoint = -1;
}

void InlineCurveWidget::setOnCurveChanged(
    std::function<void(const AnimationCurve &)> callback) {
  m_onCurveChanged = std::move(callback);
}

} // namespace NovelMind::editor
