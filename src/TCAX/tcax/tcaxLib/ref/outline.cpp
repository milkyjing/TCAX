int Outline::moveToCallback ( FT_Vector* to, Outline* outline )
  {
    if ( outline->contour_open_ ) {
      glEnd();
    }

    outline->last_vertex_ = VertexInfo( to,
					outline->colorTess(),
					outline->textureTess() );

    glBegin( GL_LINE_LOOP );

    outline->contour_open_ = true;

    return 0;
  }

  int Outline::lineToCallback ( FT_Vector* to, Outline* outline )
  {
    outline->last_vertex_ = VertexInfo( to,
					outline->colorTess(),
					outline->textureTess() );
    GLdouble g[2];

    g[X] = outline->last_vertex_.v_[X] * outline->vector_scale_;
    g[Y] = outline->last_vertex_.v_[Y] * outline->vector_scale_;

    glVertex2dv( g );

    return 0;
  }

  int Outline::conicToCallback ( FT_Vector* control, FT_Vector* to, Outline* outline )
  {
    // This is crude: Step off conics with a fixed number of increments

    VertexInfo to_vertex( to, outline->colorTess(), outline->textureTess() );
    VertexInfo control_vertex( control, outline->colorTess(), outline->textureTess() );

    double b[2], c[2], d[2], f[2], df[2], d2f[2];
    GLdouble g[3];

    g[Z] = 0.;

    b[X] = outline->last_vertex_.v_[X] - 2 * control_vertex.v_[X] +
      to_vertex.v_[X];
    b[Y] = outline->last_vertex_.v_[Y] - 2 * control_vertex.v_[Y] +
      to_vertex.v_[Y];

    c[X] = -2 * outline->last_vertex_.v_[X] + 2 * control_vertex.v_[X];
    c[Y] = -2 * outline->last_vertex_.v_[Y] + 2 * control_vertex.v_[Y];

    d[X] = outline->last_vertex_.v_[X];
    d[Y] = outline->last_vertex_.v_[Y];

    f[X] = d[X];
    f[Y] = d[Y];
    df[X] = c[X] * outline->delta_ + b[X] * outline->delta2_;
    df[Y] = c[Y] * outline->delta_ + b[Y] * outline->delta2_;
    d2f[X] = 2 * b[X] * outline->delta2_;
    d2f[Y] = 2 * b[Y] * outline->delta2_;

    for ( unsigned int i = 0; i < outline->tessellation_steps_-1; i++ ) {

      f[X] += df[X];
      f[Y] += df[Y];

      g[X] = f[X] * outline->vector_scale_;
      g[Y] = f[Y] * outline->vector_scale_;

      if ( outline->colorTess() )
	glColor4fv( outline->colorTess()->color( g ) );

      glVertex2dv( g );

      df[X] += d2f[X];
      df[Y] += d2f[Y];
    }

    g[X] = to_vertex.v_[X] * outline->vector_scale_;
    g[Y] = to_vertex.v_[Y] * outline->vector_scale_;

    if ( outline->colorTess() )
      glColor4fv( outline->colorTess()->color( g ) );

    glVertex2dv( g );

    outline->last_vertex_ = to_vertex;

    return 0;
  }

  int Outline::cubicToCallback ( FT_Vector* control1, FT_Vector* control2,
				 FT_Vector* to, Outline* outline )
  {
    // This is crude: Step off cubics with a fixed number of increments

    VertexInfo to_vertex( to, outline->colorTess(), outline->textureTess() );
    VertexInfo control1_vertex( control1, outline->colorTess(), outline->textureTess() );
    VertexInfo control2_vertex( control2, outline->colorTess(), outline->textureTess() );

    double a[2], b[2], c[2], d[2], f[2], df[2], d2f[2], d3f[2];
    GLdouble g[3];

    g[Z] = 0.;

    a[X] = -outline->last_vertex_.v_[X] + 3 * control1_vertex.v_[X]
      -3 * control2_vertex.v_[X] + to_vertex.v_[X];
    a[Y] = -outline->last_vertex_.v_[Y] + 3 * control1_vertex.v_[Y]
      -3 * control2_vertex.v_[Y] + to_vertex.v_[Y];

    b[X] = 3 * outline->last_vertex_.v_[X] - 6 * control1_vertex.v_[X] +
      3 * control2_vertex.v_[X];
    b[Y] = 3 * outline->last_vertex_.v_[Y] - 6 * control1_vertex.v_[Y] +
      3 * control2_vertex.v_[Y];

    c[X] = -3 * outline->last_vertex_.v_[X] + 3 * control1_vertex.v_[X];
    c[Y] = -3 * outline->last_vertex_.v_[Y] + 3 * control1_vertex.v_[Y];

    d[X] = outline->last_vertex_.v_[X];
    d[Y] = outline->last_vertex_.v_[Y];

    f[X] = d[X];
    f[Y] = d[Y];
    df[X] = c[X] * outline->delta_ + b[X] * outline->delta2_
      + a[X] * outline->delta3_;
    df[Y] = c[Y] * outline->delta_ + b[Y] * outline->delta2_
      + a[Y] * outline->delta3_;
    d2f[X] = 2 * b[X] * outline->delta2_ + 6 * a[X] * outline->delta3_;
    d2f[Y] = 2 * b[Y] * outline->delta2_ + 6 * a[Y] * outline->delta3_;
    d3f[X] = 6 * a[X] * outline->delta3_;
    d3f[Y] = 6 * a[Y] * outline->delta3_;

    for ( unsigned int i = 0; i < outline->tessellation_steps_-1; i++ ) {

      f[X] += df[X];
      f[Y] += df[Y];

      g[X] = f[X] * outline->vector_scale_;
      g[Y] = f[Y] * outline->vector_scale_;

      if ( outline->colorTess() )
	glColor4fv( outline->colorTess()->color( g ) );

      glVertex2dv( g );

      df[X] += d2f[X];
      df[Y] += d2f[Y];
      d2f[X] += d3f[X];
      d2f[Y] += d3f[Y];
    }

    g[X] = to_vertex.v_[X] * outline->vector_scale_;
    g[Y] = to_vertex.v_[Y] * outline->vector_scale_;

    if ( outline->colorTess() )
      glColor4fv( outline->colorTess()->color( g ) );

    glVertex2dv( g );

    outline->last_vertex_ = to_vertex;

    return 0;
  }