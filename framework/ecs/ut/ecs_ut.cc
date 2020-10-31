// *************************************************************
// File:    ecs_ut.cc
// Author:  Novoselov Anton @ 2018
// URL:     https://github.com/ans-hub/gdm_framework
// *************************************************************

#include "3rdparty/catch/catch.hpp"

#include "ecs/core.h"
#include "ecs/manager.h"
#include "ecs/component.h"
#include "ecs/system.h"

struct A : ecs::Component<ECS_COMPONENT_IDX>
{
  int value;
};
ECS_COMPONENT_REGISTER(A)

struct B : ecs::Component<ECS_COMPONENT_IDX>
{
  B() = default;
  B(int val) : value {val} {}
  int value;
};
ECS_COMPONENT_REGISTER(B)

static void a_es(A& a)
{
  a.value += 42;
}
ECS_SYSTEM_REGISTER(a_es, A)

static void b_es(const A& a, B& b)
{
  b.value += a.value;
}
ECS_SYSTEM_REGISTER(b_es, A, B)

ecs::EntityManager& g_mgr = ecs::EntityManager::GetInstance();

TEST_CASE("EntityManager")
{
   SECTION("Create entity")
   {
     auto eid = g_mgr.CreateEntity<A,B>();
     g_mgr.Tick(0.f);
     auto compA = g_mgr.GetComponent<A>(eid);
     auto compB = g_mgr.GetComponent<B>(eid);
     CHECK(compA.value == 42);
     CHECK(compB.value == 42);
   }
}