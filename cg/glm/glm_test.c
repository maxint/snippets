int main()
{
	GLMmodel *model = glmReadOBJ(fileName);
	if (model == NULL)
	{
		fprintf(stderr, "CXFaceDemoDoc::LoadModel() fail: can't load obj model");
		return FALSE;
	}
	glmUnitize(model);
	if (!model->normals)
	{
		glmFacetNormals(model);
		glmVertexNormals(model, 90.0f);
	}
}
