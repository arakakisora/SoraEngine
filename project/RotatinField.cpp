#include "RotatinField.h"
#include "ParticleMnager.h"

void RotationField::Apply(Particle& particle, float dt)
{
    Vector3 center = particle.center; // 固定中心を使う
    Vector3 toParticle = particle.transform.translate - center;

    float currentDistance = MyMath::Length(toParticle);
    Vector3 direction = MyMath::Normlize(toParticle);

    Matrix4x4 rot = MyMath::MakeRotateYMatrix(angularSpeed * dt);
    Vector3 rotatedDir = MyMath::TransformNormal(direction, rot);

   // float targetRadius = particle.radius;
    particle.transform.translate = center + rotatedDir*1.5f;
}
