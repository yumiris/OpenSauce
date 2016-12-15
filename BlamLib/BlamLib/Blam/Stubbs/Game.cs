/*
	BlamLib: .NET SDK for the Blam Engine

	See license\BlamLib\BlamLib for specific license information
*/
using System;
using System.Threading;
using TI = BlamLib.TagInterface;

namespace BlamLib.Blam.Stubbs
{
	/// <summary>
	/// Stubbs game definition implementation
	/// </summary>
	public sealed class GameDefinition : Managers.BlamDefinition, Managers.IScriptingController, Managers.IVertexBufferController
	{
		#region Implementation
		public override TagInterface.TagGroupCollection TagGroups			{ get { return Stubbs.TagGroups.Groups; } }
		//public override TI.TagGroupCollection TagGroupsInvalidForCacheViewer{ get { return Stubbs.TagGroups.GroupsInvalidForCacheViewer; } }
		//public override TI.TagGroupCollection TagGroupsInvalidForExtraction	{ get { return Stubbs.TagGroups.GroupsInvalidForExtraction; } }

		/// <remarks>Explicit resource identifying. Yes, there are reasons for this. Ask km00 if you care that much</remarks>
		internal override void IdentifyResourceProc(Managers.BlamDefinition.Game owner, string resource_name, string resource_path)
		{
			bool add_rsrc = false;

 			switch(owner.Engine)
 			{
				case BlamVersion.Stubbs_Xbox:
					switch (resource_name)
					{
						case Managers.BlamDefinition.ResourceScripts:
						case Managers.BlamDefinition.ResourceVertexBuffers: add_rsrc = true; break;
					}
					break;
				case BlamVersion.Stubbs_PC:
					switch (resource_name)
					{
						case Managers.BlamDefinition.ResourceScripts:
						case Managers.BlamDefinition.ResourceVertexBuffers: add_rsrc = true; break;
					}
					break;
				case BlamVersion.Stubbs_Mac:
					switch (resource_name)
					{
						case Managers.BlamDefinition.ResourceScripts:
						case Managers.BlamDefinition.ResourceVertexBuffers: add_rsrc = true; break;
					}
					break;

				default: throw new Debug.Exceptions.UnreachableException();
 			}

			if (add_rsrc)
				owner.AddResourceLocation(resource_name, resource_path);
		}

		internal protected override IGameResource PrecacheResource(Game owner, string resource_name, string r_path, string r_name)
		{
			IGameResource gr = null;
			bool result = false;

			switch (resource_name)
			{
				case Managers.BlamDefinition.ResourceScripts:
					gr = new Scripting.XmlInterface();
					result = gr.Load(r_path, r_name);
					break;

				case Managers.BlamDefinition.ResourceVertexBuffers:
					gr = new Render.VertexBufferInterface.VertexBuffersGen1();
					result = gr.Load(r_path, r_name);
					break;
			}

			if (!result && gr != null)
			{
				gr.Close();
				gr = null;
			}

			return gr;
		}

		internal protected override Blam.Cache.BuilderBase ConstructCacheBuilder(BlamVersion game)
		{
			Blam.Cache.BuilderBase cb = null;

			if ((game & BlamVersion.Stubbs) != 0)
			{
				cb = new Stubbs.Builder();
			}

			return cb;
		}

		internal protected override Blam.CacheFile LoadCacheFile(BlamVersion game, string file_path, bool is_resource)
		{
			Blam.CacheFile cf = null;

			if ((game & BlamVersion.Stubbs) != 0)
			{
				cf = new Stubbs.CacheFile(file_path);
			}

			return cf;
		}

		public override Managers.TagDatabase CreateTagDatabase() { return new Halo1.Tags.TagDatabase(BlamVersion.Stubbs); }

		protected override Managers.CacheTagDatabase CreateCacheTagDatabaseInternal(DatumIndex cache_id) { return new Stubbs.Tags.CacheTagDatabase((Stubbs.CacheFile)Program.GetCacheFile(cache_id)); }

		public override Managers.ErrorTagDatabase CreateErrorTagDatabase() { return new Halo1.Tags.ErrorTagDatabase(BlamVersion.Stubbs); }

		public override TI.TagGroup TagDatabaseGroup { get { return Halo1.TagGroups.tag_; } }
		#endregion

		internal GameDefinition() {}

		#region IScriptingController Members
		int ScriptingCacheReferencesXbox = 0,
			ScriptingCacheReferencesPC = 0,
			ScriptingCacheReferencesMac = 0;

		/// <summary>
		/// <see cref="BlamLib.Managers.IScriptingController"/>
		/// </summary>
		/// <param name="game"></param>
		/// <returns></returns>
		public bool ScriptingCacheOpen(BlamVersion game)
		{
			int count = 0;

			switch (game)
			{
				case BlamVersion.Stubbs_Xbox:	count = Interlocked.Increment(ref ScriptingCacheReferencesXbox);	break;
				case BlamVersion.Stubbs_PC:		count = Interlocked.Increment(ref ScriptingCacheReferencesPC);	break;
				case BlamVersion.Stubbs_Mac:	count = Interlocked.Increment(ref ScriptingCacheReferencesMac);	break;

				default: throw new Debug.Exceptions.UnreachableException();
			}

			if(count == 1)
			{
				base.PrecacheResource(game, Managers.BlamDefinition.ResourceScripts);
				return true;
			}
			else if (count == 0) throw new Debug.Exceptions.UnreachableException();

			return false;
		}

		/// <summary>
		/// <see cref="BlamLib.Managers.IScriptingController"/>
		/// </summary>
		/// <param name="game"></param>
		/// <returns></returns>
		public bool ScriptingCacheClose(BlamVersion game)
		{
			int count = -1;

			switch (game)
			{
				case BlamVersion.Stubbs_Xbox:	count = Interlocked.Decrement(ref ScriptingCacheReferencesXbox);	break;
				case BlamVersion.Stubbs_PC:		count = Interlocked.Decrement(ref ScriptingCacheReferencesPC);	break;
				case BlamVersion.Stubbs_Mac:	count = Interlocked.Decrement(ref ScriptingCacheReferencesMac);	break;

				default: throw new Debug.Exceptions.UnreachableException();
			}

			if(count == 0) // since it's pre-decrement assigned, it will equal to zero when nothing is using it anymore
			{
				base.CloseResource(game, Managers.BlamDefinition.ResourceScripts);
				return true;
			}
			else if (count == -1) throw new Debug.Exceptions.UnreachableException();

			return false;
		}
		#endregion

		// TODO: We need to generate vertex buffer definitions for Stubbs still (and see if we even need separate definitions for each platform)
		#region IVertexBufferController Members
		// Sure, we could use a hashtable for keeping references and such, but this uses way less memory, 
		// and allows us to use the method logic below and make sure we're not trying to implement any unsupported
		// engine variants. Savvy?
		int VertexBufferCacheReferencesXbox = 0, 
//			VertexBufferCacheReferencesMac = 0,
			VertexBufferCacheReferencesPC = 0;

		/// <summary>
		/// <see cref="BlamLib.Managers.IVertexBufferController"/>
		/// </summary>
		/// <param name="game"></param>
		/// <returns></returns>
		public bool VertexBufferCacheOpen(BlamVersion game)
		{
			int count = 0;

			switch (game)
			{
				case BlamVersion.Stubbs_Xbox:	count = Interlocked.Increment(ref VertexBufferCacheReferencesXbox);	break;
				case BlamVersion.Stubbs_Mac:	//count = Interlocked.Increment(ref VertexBufferCacheReferencesMac);break;
				case BlamVersion.Stubbs_PC:		count = Interlocked.Increment(ref VertexBufferCacheReferencesPC);	break;

				default: throw new Debug.Exceptions.UnreachableException();
			}

			if(count == 1)
			{
				base.PrecacheResource(game, Managers.BlamDefinition.ResourceVertexBuffers);
				return true;
			}
			else if (count == 0) throw new Debug.Exceptions.UnreachableException();

			return false;
		}

		/// <summary>
		/// <see cref="BlamLib.Managers.IVertexBufferController"/>
		/// </summary>
		/// <param name="game"></param>
		/// <returns></returns>
		public bool VertexBufferCacheClose(BlamVersion game)
		{
			int count = -1;

			switch (game)
			{
				case BlamVersion.Stubbs_Xbox:	count = Interlocked.Decrement(ref VertexBufferCacheReferencesXbox);	break;
				case BlamVersion.Stubbs_Mac:	//count = Interlocked.Decrement(ref VertexBufferCacheReferencesMac);break;
				case BlamVersion.Stubbs_PC:		count = Interlocked.Decrement(ref VertexBufferCacheReferencesPC);	break;

				default: throw new Debug.Exceptions.UnreachableException();
			}

			if(count == 0) // since it's pre-decrement assigned, it will equal to zero when nothing is using it anymore
			{
				base.CloseResource(game, Managers.BlamDefinition.ResourceVertexBuffers);
				return true;
			}
			else if (count == -1) throw new Debug.Exceptions.UnreachableException();

			return false;
		}
		#endregion
	};
}