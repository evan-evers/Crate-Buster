#include "common.h"
#include "colliders.h"
#include "float.h"

extern App app; //to check if debug is on for drawing hitboxes

OBBCollider* initOBBCollider(const float xHalfwidth, const float yHalfwidth, const Vector2 origin, const float angle);
void updateCollider(OBBCollider* collider, const float x, const float y, const float angle, const float w, const float h);
void displayCollider(SDL_Renderer* renderer, const SDL_Color* color, const OBBCollider* collider);
bool checkCollision(const OBBCollider* a, const OBBCollider* b);

//controls how large the + sign is that represents the origin of an OBB during debug drawing
static const int DEBUG_ORIGIN_DRAW_SIZE = 10;

//initialize collider
OBBCollider* initOBBCollider(const float xHalfwidth, const float yHalfwidth, const Vector2 origin, const float angle) {
    OBBCollider* collider = malloc(sizeof(OBBCollider));

    //setting axes[0] to equal i and axes[1] to equal j
    collider->axes[0].x = 1;
    collider->axes[0].y = 0;
    collider->axes[1].x = 0;
    collider->axes[1].y = 1;
    rotateVector(&(collider->axes[0]), &angle, VECTOR2_0);
    rotateVector(&(collider->axes[1]), &angle, VECTOR2_0);
    collider->halfwidths[0] = xHalfwidth;
    collider->halfwidths[1] = yHalfwidth;
    collider->origin = origin;

    return collider;
}

//update a collider's position, angle, and dimensions
//pass in FLT_MAX for the angle to skip updating the angle and -1 for w and/or h to skip updating those variables
//should only be called after all changes to an entity's movement have been made
void updateCollider(OBBCollider* collider, const float x, const float y, const float angle, const float w, const float h) {
    collider->origin.x = x;
    collider->origin.y = y;
    //float max used as a default value that the user is unlikely to input as an angle
    if (angle != FLT_MAX) {
        //setting axes[0] to equal i and axes[1] to equal j
        collider->axes[0].x = 1;
        collider->axes[0].y = 0;
        collider->axes[1].x = 0;
        collider->axes[1].y = 1;

        rotateVector(&collider->axes[0], &angle, VECTOR2_0);
        rotateVector(&collider->axes[1], &angle, VECTOR2_0);
    }

    //if the user overrode the default values, update the width and height
    if (w > 0)
        collider->halfwidths[0] = w;
    if (h > 0)
        collider->halfwidths[1] = h;
}

//display a collision box
void displayCollider(SDL_Renderer* renderer, const SDL_Color *color, const OBBCollider *collider) {
    //only draw this if debug has been set to true
    if (!app.debug) {
        return;
    }

    //set render draw color
    SDL_SetRenderDrawColor(renderer, color->r, color->g, color->b, color->a);

    //draw the outline of the collider

    //a collection of points representing the rectangle
    //5 points are given because SDL_RenderDrawLines draws (count - 1) lines (think of it as a graph)
    //the rectangle's points are treated as vectors and rotated using the standard rotation matrix
    SDL_Point rectangle[5] = { vec2ToSDL_Point(addVec2(addVec2(scalarMultVec2(collider->axes[0],collider->halfwidths[0]), scalarMultVec2(collider->axes[1], collider->halfwidths[1])), collider->origin)),      //bottom right point
                                vec2ToSDL_Point(addVec2(addVec2(scalarMultVec2(collider->axes[0], -collider->halfwidths[0]), scalarMultVec2(collider->axes[1], collider->halfwidths[1])), collider->origin)),     //bottom left point
                                vec2ToSDL_Point(addVec2(addVec2(scalarMultVec2(collider->axes[0], -collider->halfwidths[0]), scalarMultVec2(collider->axes[1], -collider->halfwidths[1])), collider->origin)),    //top left point
                                vec2ToSDL_Point(addVec2(addVec2(scalarMultVec2(collider->axes[0], collider->halfwidths[0]), scalarMultVec2(collider->axes[1], -collider->halfwidths[1])), collider->origin)),     //top right point
                                vec2ToSDL_Point(addVec2(addVec2(scalarMultVec2(collider->axes[0], collider->halfwidths[0]), scalarMultVec2(collider->axes[1], collider->halfwidths[1])), collider->origin)), };    //bottom right point
    SDL_RenderDrawLines(renderer, rectangle, 5);
    /*SDL_RenderDrawLine(renderer, rectangle[0].x, rectangle[0].y, collider->origin.x, collider->origin.y);
    SDL_RenderDrawLine(renderer, rectangle[1].x, rectangle[1].y, collider->origin.x, collider->origin.y);
    SDL_RenderDrawLine(renderer, rectangle[2].x, rectangle[2].y, collider->origin.x, collider->origin.y);
    SDL_RenderDrawLine(renderer, rectangle[3].x, rectangle[3].y, collider->origin.x, collider->origin.y);*/

    //more debug
    //printf("(%d, %d), (%d, %d), (%d, %d), (%d, %d)\n", rectangle[0].x, rectangle[0].y, rectangle[1].x, rectangle[1].y, rectangle[2].x, rectangle[2].y, rectangle[3].x, rectangle[3].y);

    //draw the center of rotation as a +
    SDL_RenderDrawLine(renderer, collider->origin.x - DEBUG_ORIGIN_DRAW_SIZE, collider->origin.y, collider->origin.x + DEBUG_ORIGIN_DRAW_SIZE, collider->origin.y);
    SDL_RenderDrawLine(renderer, collider->origin.x, collider->origin.y - DEBUG_ORIGIN_DRAW_SIZE, collider->origin.x, collider->origin.y + DEBUG_ORIGIN_DRAW_SIZE);

    //reset renderer draw color to black
    SDL_SetRenderDrawColor(renderer, 0xFF, 0xFF, 0xFF, 0xFF);
}

//check collision between two OBB colliders
//first, check if the distance between the centers of the two boxes is less than the sum of the "radii" of the rectangles
// (the radii being the diagonal distance from the center of the box to one of its corners)
//if the distance is less than that, use seperating axes therorem to detect a collision
bool checkCollision(const OBBCollider* a, const OBBCollider* b) {
    //radius-distance check for optimization
    //the "radius" is the distance from the center of a box to one of its corners

    //variable setup (axes must be passed in as absolute valued vectors for the radii calculations to work properly)
    Vector2* aR = malloc(sizeof(Vector2));
    Vector2* bR = malloc(sizeof(Vector2));
    Vector2* sumARBR = malloc(sizeof(Vector2));
    aR->x = fabs(a->axes[0].x) * a->halfwidths[0] + fabs(a->axes[1].x) * a->halfwidths[1];
    aR->y = fabs(a->axes[0].y) * a->halfwidths[0] + fabs(a->axes[1].y) * a->halfwidths[1];
    bR->x = fabs(b->axes[0].x) * b->halfwidths[0] + fabs(b->axes[1].x) * b->halfwidths[1];
    bR->y = fabs(b->axes[0].y) * b->halfwidths[0] + fabs(b->axes[1].y) * b->halfwidths[1];
    sumARBR->x = aR->x + bR->x;
    sumARBR->y = aR->y + bR->y;
    float* radiusSumSquared = malloc(sizeof(float));
    *radiusSumSquared = dotProduct(sumARBR, sumARBR);   //dot product of a vector with itself is its magnitude squared
    float* centerDistanceSquared = malloc(sizeof(float));
    *centerDistanceSquared = distanceSquared(a->origin.x, a->origin.y, b->origin.x, b->origin.y);

    //distance check
    if (*radiusSumSquared < *centerDistanceSquared)
        return false;

    //clean up now-unneeded variables from radius-distance check
    free(aR);
    free(bR);
    free(sumARBR);
    free(radiusSumSquared);    
    free(centerDistanceSquared);

    //store the points of each collider as vectors to use in projection
    Vector2 aCorners[4];
    aCorners[0].x = (a->axes[0].x * a->halfwidths[0] + a->axes[1].x * a->halfwidths[1]) + a->origin.x;   //bottom right point
    aCorners[0].y = (a->axes[0].y * a->halfwidths[0] + a->axes[1].y * a->halfwidths[1]) + a->origin.y;  
    aCorners[1].x = (a->axes[0].x * -a->halfwidths[0] + a->axes[1].x * a->halfwidths[1]) + a->origin.x;   //bottom left point
    aCorners[1].y = (a->axes[0].y * -a->halfwidths[0] + a->axes[1].y * a->halfwidths[1]) + a->origin.y;
    aCorners[2].x = (a->axes[0].x * -a->halfwidths[0] + a->axes[1].x * -a->halfwidths[1]) + a->origin.x;   //top left point
    aCorners[2].y = (a->axes[0].y * -a->halfwidths[0] + a->axes[1].y * -a->halfwidths[1]) + a->origin.y;
    aCorners[3].x = (a->axes[0].x * a->halfwidths[0] + a->axes[1].x * -a->halfwidths[1]) + a->origin.x;   //top right point
    aCorners[3].y = (a->axes[0].y * a->halfwidths[0] + a->axes[1].y * -a->halfwidths[1]) + a->origin.y;

    Vector2 bCorners[4];
    bCorners[0].x = (b->axes[0].x * b->halfwidths[0] + b->axes[1].x * b->halfwidths[1]) + b->origin.x;   //bottom right point
    bCorners[0].y = (b->axes[0].y * b->halfwidths[0] + b->axes[1].y * b->halfwidths[1]) + b->origin.y;
    bCorners[1].x = (b->axes[0].x * -b->halfwidths[0] + b->axes[1].x * b->halfwidths[1]) + b->origin.x;   //bottom left point
    bCorners[1].y = (b->axes[0].y * -b->halfwidths[0] + b->axes[1].y * b->halfwidths[1]) + b->origin.y;
    bCorners[2].x = (b->axes[0].x * -b->halfwidths[0] + b->axes[1].x * -b->halfwidths[1]) + b->origin.x;   //top left point
    bCorners[2].y = (b->axes[0].y * -b->halfwidths[0] + b->axes[1].y * -b->halfwidths[1]) + b->origin.y;
    bCorners[3].x = (b->axes[0].x * b->halfwidths[0] + b->axes[1].x * -b->halfwidths[1]) + b->origin.x;   //top right point
    bCorners[3].y = (b->axes[0].y * b->halfwidths[0] + b->axes[1].y * -b->halfwidths[1]) + b->origin.y;

    //values for storing dot products of projection values of both boxes
    float   aCurrentProjection, aMaxProjection, aMinProjection,
        bCurrentProjection, bMaxProjection, bMinProjection;

    //go through all four axes, checking for overlap
    //a's axes
    for (int i = 0; i < 2; i++) {
        //initialize/reset maximums and minimums
        aMaxProjection = -FLT_MAX; aMinProjection = FLT_MAX;
        bMaxProjection = -FLT_MAX; bMinProjection = FLT_MAX;
        //for each corner of each box, project that corner onto the current axis
        //then find the dot product of that projected vector with the current axis
        //the resulting value relates to the corner's position on the line
        //by comparing the maximums and minimums of those values for each box, we can determine intersection on an axis
        for (int j = 0; j < 4; j++) {
            aCurrentProjection = dotProduct(&(a->axes[i]), projectVector(&aCorners[j], &(a->axes[i])));
            //find max/min
            if (aCurrentProjection > aMaxProjection)
                aMaxProjection = aCurrentProjection;
            if (aCurrentProjection < aMinProjection)
                aMinProjection = aCurrentProjection;
        }
        for (int j = 0; j < 4; j++) {
            bCurrentProjection = dotProduct(&(a->axes[i]), projectVector(&bCorners[j], &(a->axes[i])));
            //find max/min
            if (bCurrentProjection > bMaxProjection)
                bMaxProjection = bCurrentProjection;
            if (bCurrentProjection < bMinProjection)
                bMinProjection = bCurrentProjection;
        }

        //check for overlap of points on the axis projected to
        //if there's no overlap on even a single axis, there's no collision
        if (aMinProjection > bMaxProjection)
            return false;
        if (bMinProjection > aMaxProjection)
            return false;
    }
    //b's axes
    for (int i = 0; i < 2; i++) {
        //reset maximums and minimums
        aMaxProjection = -FLT_MAX; aMinProjection = FLT_MAX;
        bMaxProjection = -FLT_MAX; bMinProjection = FLT_MAX;
        //for each corner of each box, project that corner onto the current axis
        //then find the dot product of that projected vector with the current axis
        //the resulting value relates to the corner's position on the line
        //by comparing the maximums and minimums of those values for each box, we can determine intersection on an axis
        for (int j = 0; j < 4; j++) {
            aCurrentProjection = dotProduct(&(b->axes[i]), projectVector(&aCorners[j], &(b->axes[i])));
            //find max/min
            if (aCurrentProjection > aMaxProjection)
                aMaxProjection = aCurrentProjection;
            if (aCurrentProjection < aMinProjection)
                aMinProjection = aCurrentProjection;
        }
        for (int j = 0; j < 4; j++) {
            bCurrentProjection = dotProduct(&(b->axes[i]), projectVector(&bCorners[j], &(b->axes[i])));
            //find max/min
            if (bCurrentProjection > bMaxProjection)
                bMaxProjection = bCurrentProjection;
            if (bCurrentProjection < bMinProjection)
                bMinProjection = bCurrentProjection;
        }

        //check for overlap of points on the axis projected to
        //if there's no overlap on even a single axis, there's no collision
        if (aMinProjection > bMaxProjection)
            return false;
        if (bMinProjection > aMaxProjection)
            return false;
    }

    //there's overlap on all four axes; collision detected.
    return true;
}

//deallocate collider
void deleteCollider(OBBCollider *collider) {
    free(collider);
}