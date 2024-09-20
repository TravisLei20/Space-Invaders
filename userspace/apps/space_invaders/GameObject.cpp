
#include <stdint.h>

#include "Colors.h"
#include "GameObject.h"
#include "Globals.h"
#include "Graphics.h"
#include "Sprite.h"

// This is a base class for all objects in the game.

GameObject::GameObject(Sprite *sprite, uint16_t x, uint16_t y, uint8_t size,
                       rgb_t color) {
  this->sprite = sprite;
  this->x = x;
  this->y = y;
  this->size = size;
  this->color = color;
  this->alive = true;
}

// Bring object back to life.  Make alive and redraw.
void GameObject::resurrect(Sprite * new_sprite) {
  this->sprite = new_sprite;
  alive = true;
  draw();
}

// Erase and redraw an object, at a new location and/or with a new sprite.
//  - newSprite: new sprite to draw, or nullptr to not change the sprite.
//  - delta_x/y: move object, input 0 to leave in same spot.
void GameObject::move(Sprite *newSprite, int16_t delta_x, int16_t delta_y) {

  

  if (this->isAlive()) {
    // Erase the old sprite
    erase();
    // Update the x and y coordinates, and the sprite (for aliens)
    this->x += delta_x;
    this->y += delta_y;
    this->sprite = newSprite;

    // Draw the new sprite
    draw();
  }
}

// Draw the object
void GameObject::draw() {

  // Only draw if alive
  if (alive) {
    Globals::getGraphics().drawSprite(sprite, x, y, size, color,
                                      Globals::getBackgroundColor());
  }
}

// Erase the object
void GameObject::erase() {
  Globals::getGraphics().drawSprite(sprite, x, y, size,
                                    Globals::getBackgroundColor(),
                                    Globals::getBackgroundColor());
}

// Draw the object, but don't draw the background pixels (this is slower, but
// is needed for the bunker damage)
void GameObject::drawNoBackground() {
  Globals::getGraphics().drawSprite(sprite, x, y, size, color);
}

// Kill the object, which should erase it.
void GameObject::kill() {
  alive = false;
  erase();
}

bool between(uint16_t value, uint16_t side1, uint16_t side2) {
  if (value > side1) {
    if (value < side2) {
      return true;
    }
  }
  return false;
}

// Check if this object is overlapping a given object.  This can be used to
// handle all collisions in the game.
bool GameObject::isOverlapping(GameObject *object) {
  if (object == nullptr) {
    return false;
  }

  if (this->isAlive() && object->isAlive()) {
    uint16_t left1 = this->getX();
    uint16_t right1 = this->getX() + this->getWidth();
    uint16_t top1 = this->getY();
    uint16_t bottom1 = this->getY() + this->getHeight();

    uint16_t left2 = object->getX();
    uint16_t right2 = object->getX() + object->getWidth();
    uint16_t top2 = object->getY();
    uint16_t bottom2 = object->getY() + object->getHeight();
    

    if (between(left1, left2, right2) || between(right1, left2, right2)) {
      if (between(top1, top2, bottom2) || between(bottom1, top2, bottom2)) {
        return true;
      }
    }
    if (between(left2, left1, right1) || between(right2, left1, right1)) {
      if (between(top2, top1, bottom1) || between(bottom2, top1, bottom1)) {
        return true;
      }
    }

  }
  return false;
}
